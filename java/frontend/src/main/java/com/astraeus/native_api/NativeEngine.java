package com.astraeus.native_api;

import com.astraeus.native_api.lifecycle.EngineConfig;
import com.astraeus.native_api.model.FrameStats;
import com.astraeus.native_api.model.PassTiming;
import com.astraeus.native_api.model.PickResult;
import com.astraeus.native_api.model.PixelBufferView;

import java.lang.foreign.*;
import java.lang.invoke.VarHandle;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/**
 * High-level Java wrapper around the native Astraeus engine.
 * 
 * <p>This class provides a typed, ergonomic API for interacting with the native
 * C++ engine core. All MemorySegment details are encapsulated and exposed through
 * typed value objects in the {@link com.astraeus.native_api.model} package.</p>
 * 
 * <p><b>Thread Safety:</b> This class is NOT thread-safe. All methods must be called
 * from the same thread (typically the JavaFX Application Thread) unless otherwise noted.
 * The engine maintains internal state that is not protected by synchronization.</p>
 * 
 * <p><b>Lifecycle:</b> The engine must be explicitly closed when no longer needed by
 * calling {@link #close()}. This frees native resources and GPU memory. Failure to close
 * will result in resource leaks.</p>
 * 
 * <p><b>Usage:</b></p>
 * <pre>{@code
 * // Create engine
 * try (NativeEngine engine = new NativeEngine(1280, 720, true)) {
 *     // Configure readback
 *     engine.configureReadback(2560, 1440, false);
 *     
 *     // Render loop
 *     while (running) {
 *         engine.beginFrame(deltaTime);
 *         // ... update scene ...
 *         engine.endFrame();
 *         
 *         // Get rendered output
 *         PixelBufferView colorBuffer = engine.getColorBuffer();
 *         // ... display buffer ...
 *     }
 * }
 * }</pre>
 * 
 * @see com.astraeus.native_api.lifecycle.EngineConfig
 * @see com.astraeus.native_api.model.PixelBufferView
 * @see com.astraeus.native_api.model.FrameStats
 * @see com.astraeus.native_api.model.PickResult
 */
public class NativeEngine implements AutoCloseable {
    
    private MemorySegment engineHandle;
    private final Arena arena;
    private boolean closed = false;
    
    /**
     * Create a new engine instance with default configuration.
     * 
     * <p><b>Thread Safety:</b> Must be called from the thread that will own the engine
     * (typically JavaFX Application Thread).</p>
     * 
     * @param width Initial viewport width in pixels
     * @param height Initial viewport height in pixels
     * @param enableValidation Enable validation layers for debugging
     * @throws RuntimeException if engine creation fails
     */
    public NativeEngine(int width, int height, boolean enableValidation) {
        this(new EngineConfig()
            .setInitialWidth(width)
            .setInitialHeight(height)
            .setEnableValidation(enableValidation));
    }
    
    /**
     * Create a new engine instance with custom configuration.
     * 
     * <p><b>Thread Safety:</b> Must be called from the thread that will own the engine
     * (typically JavaFX Application Thread).</p>
     * 
     * @param engineConfig Engine configuration
     * @throws RuntimeException if engine creation fails
     * @throws NullPointerException if engineConfig is null
     */
    public NativeEngine(EngineConfig engineConfig) {
        if (engineConfig == null) {
            throw new NullPointerException("engineConfig cannot be null");
        }
        
        this.arena = Arena.ofShared();
        
        // Allocate and populate EngineConfig using layout accessors
        MemorySegment config = arena.allocate(EngineBindings.ENGINE_CONFIG_LAYOUT);
        
        // Use field offsets from layout for safe, platform-independent access
        VarHandle widthHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("initial_width"));
        VarHandle heightHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("initial_height"));
        VarHandle validationHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("enable_validation"));
        VarHandle debugHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("enable_debug_output"));
        VarHandle logPathHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("log_file_path"));
        
        widthHandle.set(config, 0L, engineConfig.getInitialWidth());
        heightHandle.set(config, 0L, engineConfig.getInitialHeight());
        validationHandle.set(config, 0L, engineConfig.isEnableValidation());
        debugHandle.set(config, 0L, engineConfig.isEnableDebugOutput());
        
        // Handle log file path (null-safe)
        MemorySegment logPathSegment = MemorySegment.NULL;
        if (engineConfig.getLogFilePath() != null && !engineConfig.getLogFilePath().isEmpty()) {
            logPathSegment = arena.allocateFrom(engineConfig.getLogFilePath(), StandardCharsets.UTF_8);
        }
        logPathHandle.set(config, 0L, logPathSegment);
        
        try {
            // Call native function
            engineHandle = (MemorySegment) EngineBindings.CREATE_ENGINE.invoke(config);
            
            if (engineHandle == null || engineHandle.equals(MemorySegment.NULL)) {
                throw new RuntimeException("Failed to create native engine");
            }
            
            // Verify engine is valid
            boolean valid = (boolean) EngineBindings.IS_VALID.invoke(engineHandle);
            if (!valid) {
                throw new RuntimeException("Native engine is not valid after creation");
            }
            
            System.out.println("[NativeEngine] Engine created successfully");
            
        } catch (Throwable e) {
            arena.close();
            throw new RuntimeException("Failed to initialize native engine", e);
        }
    }
    
    /**
     * Begin a new frame.
     * @param deltaTime Time since last frame in seconds
     * @return true on success, false on failure
     */
    public boolean beginFrame(double deltaTime) {
        checkClosed();
        try {
            int result = (int) EngineBindings.BEGIN_FRAME.invoke(engineHandle, deltaTime);
            return result == EngineBindings.ASTRAEUS_SUCCESS;
        } catch (Throwable e) {
            throw new RuntimeException("Failed to begin frame", e);
        }
    }
    
    /**
     * End the current frame.
     * @return true on success, false on failure
     */
    public boolean endFrame() {
        checkClosed();
        try {
            int result = (int) EngineBindings.END_FRAME.invoke(engineHandle);
            return result == EngineBindings.ASTRAEUS_SUCCESS;
        } catch (Throwable e) {
            throw new RuntimeException("Failed to end frame", e);
        }
    }
    
    /**
     * Resize the viewport.
     * IMPORTANT: This only changes the viewport region, NOT the backing buffer size.
     * @param width New viewport width
     * @param height New viewport height
     */
    public void resizeViewport(int width, int height) {
        checkClosed();
        try {
            EngineBindings.RESIZE_VIEWPORT.invoke(engineHandle, width, height);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to resize viewport", e);
        }
    }
    
    /**
     * Configure readback buffers with fixed backing size.
     * Must be called before first frame to set maximum buffer sizes.
     * @param maxWidth Maximum expected viewport width
     * @param maxHeight Maximum expected viewport height
     * @param enableDoubleBuffer Enable double-buffered readback (safer, slightly slower)
     */
    public void configureReadback(int maxWidth, int maxHeight, boolean enableDoubleBuffer) {
        checkClosed();
        try {
            // Allocate ReadbackConfig structs
            MemorySegment colorConfig = arena.allocate(EngineBindings.READBACK_CONFIG_LAYOUT);
            MemorySegment idConfig = arena.allocate(EngineBindings.READBACK_CONFIG_LAYOUT);
            
            // Get field handles
            VarHandle maxWidthHandle = EngineBindings.READBACK_CONFIG_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("max_width"));
            VarHandle maxHeightHandle = EngineBindings.READBACK_CONFIG_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("max_height"));
            VarHandle formatHandle = EngineBindings.READBACK_CONFIG_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("format"));
            VarHandle doubleBufferHandle = EngineBindings.READBACK_CONFIG_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("enable_double_buffer"));
            
            // Configure color buffer (BGRA8 format)
            maxWidthHandle.set(colorConfig, 0L, maxWidth);
            maxHeightHandle.set(colorConfig, 0L, maxHeight);
            formatHandle.set(colorConfig, 0L, EngineBindings.PIXEL_FORMAT_BGRA8);
            doubleBufferHandle.set(colorConfig, 0L, enableDoubleBuffer);
            
            // Configure ID buffer (R32UI format)
            maxWidthHandle.set(idConfig, 0L, maxWidth);
            maxHeightHandle.set(idConfig, 0L, maxHeight);
            formatHandle.set(idConfig, 0L, EngineBindings.PIXEL_FORMAT_R32UI);
            doubleBufferHandle.set(idConfig, 0L, enableDoubleBuffer);
            
            // Call native function
            boolean success = (boolean) EngineBindings.CONFIGURE_READBACK.invoke(
                engineHandle, colorConfig, idConfig);
            
            if (!success) {
                throw new RuntimeException("Failed to configure readback buffers");
            }
            
            System.out.println("[NativeEngine] Readback configured: " + maxWidth + "x" + 
                             maxHeight + " (double_buffer=" + enableDoubleBuffer + ")");
            
        } catch (Throwable e) {
            throw new RuntimeException("Failed to configure readback", e);
        }
    }
    
    /**
     * Get the color buffer view for rendering.
     * The returned PixelBufferView has a stable data pointer that never changes.
     * Only the viewport dimensions (width/height) change on resize.
     * 
     * SAFETY: The ByteBuffer returned is the STABLE, FULL-CAPACITY buffer for JavaFX PixelBuffer.
     * Its position/limit state is NEVER mutated after initial creation.
     * This buffer should be given to JavaFX PixelBuffer and never modified.
     * 
     * THREAD-SAFETY: This method is NOT thread-safe. It must be called from a single thread only
     * (typically the JavaFX application thread). Concurrent calls can lead to race conditions
     * when updating the buffer state.
     */
    // Cached backing for JavaFX PixelBuffer (STABLE, IMMUTABLE STATE)
    private MemorySegment colorDataSeg;
    private ByteBuffer colorByteBufferStable;  // For JavaFX - never mutate position/limit after creation
    private long colorAddr = 0;
    private int colorBackingSize = 0;

    public PixelBufferView getColorBuffer() {
        checkClosed();

        // Out struct is short-lived; that's fine (just metadata)
        MemorySegment viewStruct = arena.allocate(EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT);

        try {
            EngineBindings.GET_COLOR_BUFFER.invoke(engineHandle, viewStruct);
        } catch (Throwable t) {
            throw new RuntimeException("GET_COLOR_BUFFER failed", t);
        }

        PixelBufferView view = new PixelBufferView(viewStruct);

        long addr = view.dataAddress();
        int backingSize = view.getMaxBackingSize();
        int stride = view.getStride();
        int maxWidth = view.getMaxBackingWidth();
        int expectedStride = maxWidth * 4; // BGRA8 = 4 bytes per pixel
        
        // Calculate needed size with overflow detection
        int needed;
        try {
            needed = Math.multiplyExact(view.getStride(), view.getHeight());
        } catch (ArithmeticException e) {
            throw new IllegalStateException("Viewport size calculation overflowed: stride=" + 
                    view.getStride() + " * height=" + view.getHeight() + " exceeds Integer.MAX_VALUE", e);
        }

        if (addr == 0 || backingSize <= 0) {
            throw new IllegalStateException("Invalid color view: addr=" + addr + " backingSize=" + backingSize);
        }
        if (needed > backingSize) {
            throw new IllegalStateException("Color view size mismatch: needed=" + needed + " backingSize=" + backingSize +
                    " stride=" + view.getStride() + " height=" + view.getHeight());
        }
        
        // Validate stride matches expected value
        if (stride != expectedStride) {
            System.err.println("[NativeEngine] WARNING: Stride mismatch in color buffer!");
            System.err.println("  Expected stride: " + expectedStride + " bytes/row (max_width=" + maxWidth + ")");
            System.err.println("  Actual stride: " + stride + " bytes/row");
            System.err.println("  This may cause rendering artifacts!");
        }

        // Rebuild only if address/size changed
        if (colorByteBufferStable == null || colorAddr != addr || colorBackingSize != backingSize) {
            colorAddr = addr;
            colorBackingSize = backingSize;

            // IMPORTANT: attach to engine lifetime arena/session (this.arena)
            colorDataSeg = MemorySegment.ofAddress(addr).reinterpret(backingSize, arena, null);
            
            // Create STABLE buffer for JavaFX with position=0, limit=capacity
            // This buffer's state will NEVER be mutated after creation
            colorByteBufferStable = colorDataSeg.asByteBuffer();
            colorByteBufferStable.position(0);
            colorByteBufferStable.limit(colorByteBufferStable.capacity());
            
            System.out.println("[NativeEngine] Color buffer initialized:");
            System.out.println("  Address: 0x" + Long.toHexString(addr));
            System.out.println("  Capacity: " + colorByteBufferStable.capacity() + " bytes");
            System.out.println("  Backing size: " + backingSize + " bytes");
        }

        // CRITICAL: Return the stable buffer that JavaFX owns
        // DO NOT mutate its position/limit after this point
        // If you need a sized view internally, call view.getViewportBuffer() instead
        view.attachByteBuffer(colorByteBufferStable, needed);
        return view;
    }



    /**
     * Get the ID buffer view for picking.
     * The returned PixelBufferView has a stable data pointer that never changes.
     * Only the viewport dimensions (width/height) change on resize.
     * 
     * SAFETY: The ByteBuffer returned is the STABLE, FULL-CAPACITY buffer.
     * Its position/limit state is NEVER mutated after initial creation.
     * 
     * THREAD-SAFETY: This method is NOT thread-safe. It must be called from a single thread only
     * (typically the JavaFX application thread). Concurrent calls can lead to race conditions
     * when updating the buffer state.
     */
    private MemorySegment idDataSeg;
    private ByteBuffer idByteBufferStable;  // STABLE buffer - never mutate position/limit after creation
    private long idAddr = 0;
    private int idBackingSize = 0;

    public PixelBufferView getIdBuffer() {
        checkClosed();

        MemorySegment viewStruct = arena.allocate(EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT);

        try {
            EngineBindings.GET_ID_BUFFER.invoke(engineHandle, viewStruct);
        } catch (Throwable t) {
            throw new RuntimeException("GET_ID_BUFFER failed", t);
        }

        PixelBufferView view = new PixelBufferView(viewStruct);

        long addr = view.dataAddress();
        int backingSize = view.getMaxBackingSize();
        
        // Calculate needed size with overflow detection
        int needed;
        try {
            needed = Math.multiplyExact(view.getStride(), view.getHeight());
        } catch (ArithmeticException e) {
            throw new IllegalStateException("ID buffer size calculation overflowed: stride=" + 
                    view.getStride() + " * height=" + view.getHeight() + " exceeds Integer.MAX_VALUE", e);
        }

        if (addr == 0 || backingSize <= 0) {
            throw new IllegalStateException("Invalid id view: addr=" + addr + " backingSize=" + backingSize);
        }
        if (needed > backingSize) {
            throw new IllegalStateException("ID view size mismatch: needed=" + needed + " backingSize=" + backingSize +
                    " stride=" + view.getStride() + " height=" + view.getHeight());
        }

        if (idByteBufferStable == null || idAddr != addr || idBackingSize != backingSize) {
            idAddr = addr;
            idBackingSize = backingSize;

            idDataSeg = MemorySegment.ofAddress(addr).reinterpret(backingSize, arena, null);
            
            // Create STABLE buffer with position=0, limit=capacity
            // This buffer's state will NEVER be mutated after creation
            idByteBufferStable = idDataSeg.asByteBuffer();
            idByteBufferStable.position(0);
            idByteBufferStable.limit(idByteBufferStable.capacity());
        }

        // Return the stable buffer
        // DO NOT mutate its position/limit after this point
        view.attachByteBuffer(idByteBufferStable, needed);
        return view;
    }


    /**
     * Perform picking at screen coordinates.
     * 
     * <p>Returns information about the entity at the specified screen position.
     * Uses the ID buffer to determine which entity (if any) was clicked.</p>
     * 
     * <p><b>Thread Safety:</b> Must be called from the owning thread only.</p>
     * 
     * @param screenX Screen X coordinate (0 = left edge)
     * @param screenY Screen Y coordinate (0 = top edge)
     * @return PickResult containing pick result data (never null)
     * @throws RuntimeException if picking fails
     */
    public PickResult pick(int screenX, int screenY) {
        checkClosed();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment out = arena.allocate(EngineBindings.PICK_RESULT_LAYOUT);
            EngineBindings.PICK.invoke(engineHandle, screenX, screenY, out);

            return new PickResult(out);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to perform picking", e);
        }
    }
    
    /**
     * Enable or disable telemetry collection.
     * @param enabled Whether to enable telemetry
     */
    public void enableTelemetry(boolean enabled) {
        checkClosed();
        try {
            EngineBindings.ENABLE_TELEMETRY.invoke(engineHandle, enabled);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to enable/disable telemetry", e);
        }
    }
    
    /**
     * Check if telemetry is enabled.
     * @return true if telemetry is enabled
     */
    public boolean isTelemetryEnabled() {
        checkClosed();
        try {
            return (boolean) EngineBindings.IS_TELEMETRY_ENABLED.invoke(engineHandle);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to check telemetry status", e);
        }
    }
    
    /**
     * Get current frame telemetry statistics.
     * 
     * <p><b>Note:</b> Telemetry must be enabled via {@link #enableTelemetry(boolean)}
     * for this method to return meaningful data.</p>
     * 
     * <p><b>Thread Safety:</b> Must be called from the owning thread only.</p>
     * 
     * @return FrameStats for the current frame (never null)
     * @throws RuntimeException if telemetry retrieval fails
     * @see #enableTelemetry(boolean)
     * @see #isTelemetryEnabled()
     */
    public FrameStats getTelemetryStats() {
        checkClosed();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment out = arena.allocate(EngineBindings.TELEMETRY_FRAME_STATS_LAYOUT);
            EngineBindings.GET_TELEMETRY_FRAME_STATS.invoke(engineHandle, out);
            
            return new FrameStats(out);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get telemetry stats", e);
        }
    }
    
    /**
     * Get telemetry history (ring buffer of recent frames).
     * 
     * <p>Returns up to maxFrames recent frame statistics, ordered from most
     * recent to oldest.</p>
     * 
     * <p><b>Thread Safety:</b> Must be called from the owning thread only.</p>
     * 
     * @param maxFrames Maximum number of frames to retrieve
     * @return List of FrameStats (most recent first, never null)
     * @throws RuntimeException if telemetry history retrieval fails
     */
    public java.util.List<FrameStats> getTelemetryHistory(int maxFrames) {
        checkClosed();
        try (Arena arena = Arena.ofConfined()) {
            // Allocate buffer for frame stats array
            long statsSize = EngineBindings.TELEMETRY_FRAME_STATS_LAYOUT.byteSize();
            MemorySegment buffer = arena.allocate(statsSize * maxFrames);
            
            int framesWritten = (int) EngineBindings.GET_TELEMETRY_HISTORY.invoke(
                engineHandle, buffer, maxFrames);
            
            java.util.List<FrameStats> history = new java.util.ArrayList<>(framesWritten);
            for (int i = 0; i < framesWritten; i++) {
                MemorySegment statsSegment = buffer.asSlice(i * statsSize, statsSize);
                history.add(new FrameStats(statsSegment));
            }
            
            return history;
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get telemetry history", e);
        }
    }
    
    /**
     * Get the number of render passes in the current frame.
     * @return Number of render passes
     */
    public int getPassCount() {
        checkClosed();
        try {
            return (int) EngineBindings.GET_PASS_COUNT.invoke(engineHandle);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get pass count", e);
        }
    }
    
    /**
     * Get timing information for a specific render pass.
     * @param passIndex Index of the pass (0 to passCount-1)
     * @return PassTiming with name and time, or null if pass doesn't exist
     */
    public PassTiming getPassTiming(int passIndex) {
        checkClosed();
        try (Arena arena = Arena.ofConfined()) {
            // Allocate buffer for pass name (max 64 chars)
            MemorySegment nameBuffer = arena.allocate(64);
            MemorySegment timeMs = arena.allocate(ValueLayout.JAVA_DOUBLE);
            
            boolean success = (boolean) EngineBindings.GET_PASS_TIMING.invoke(
                engineHandle, passIndex, nameBuffer, 64, timeMs);
            
            if (!success) {
                return null;
            }
            
            // Read pass name (null-terminated string)
            String passName = nameBuffer.getString(0, StandardCharsets.UTF_8);
            double time = timeMs.get(ValueLayout.JAVA_DOUBLE, 0);
            
            return new PassTiming(passName, time);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get pass timing", e);
        }
    }


    /**
     * Create a new entity.
     * @return Entity ID
     */
    public int createEntity() {
        checkClosed();
        try {
            return (int) EngineBindings.CREATE_ENTITY.invoke(engineHandle);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to create entity", e);
        }
    }
    
    /**
     * Destroy an entity.
     * @param entityId Entity ID
     */
    public void destroyEntity(int entityId) {
        checkClosed();
        try {
            EngineBindings.DESTROY_ENTITY.invoke(engineHandle, entityId);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to destroy entity", e);
        }
    }
    
    /**
     * Set entity transform (position, rotation, scale).
     * @param entityId Entity ID
     * @param posX Position X
     * @param posY Position Y
     * @param posZ Position Z
     * @param rotX Rotation X (radians)
     * @param rotY Rotation Y (radians)
     * @param rotZ Rotation Z (radians)
     * @param scaleX Scale X
     * @param scaleY Scale Y
     * @param scaleZ Scale Z
     */
    public void setEntityTransform(int entityId, float posX, float posY, float posZ,
                                   float rotX, float rotY, float rotZ,
                                   float scaleX, float scaleY, float scaleZ) {
        checkClosed();
        try {
            EngineBindings.SET_ENTITY_TRANSFORM.invoke(engineHandle, entityId,
                posX, posY, posZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to set entity transform", e);
        }
    }
    
    /**
     * Set entity renderable (visibility) state.
     * @param entityId Entity ID
     * @param visible Whether entity should be rendered
     */
    public void setEntityRenderable(int entityId, boolean visible) {
        checkClosed();
        try {
            EngineBindings.SET_ENTITY_RENDERABLE.invoke(engineHandle, entityId, visible);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to set entity renderable state", e);
        }
    }
    
    /**
     * Set entity color.
     * @param entityId Entity ID
     * @param r Red component [0-1]
     * @param g Green component [0-1]
     * @param b Blue component [0-1]
     * @param a Alpha component [0-1]
     */
    public void setEntityColor(int entityId, float r, float g, float b, float a) {
        checkClosed();
        try {
            EngineBindings.SET_ENTITY_COLOR.invoke(engineHandle, entityId, r, g, b, a);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to set entity color", e);
        }
    }
    
    /**
     * Set entity trail (enable trail rendering with specified max length).
     * @param entityId Entity ID
     * @param maxPoints Maximum number of trail points
     */
    public void setEntityTrail(int entityId, int maxPoints) {
        checkClosed();
        try {
            EngineBindings.SET_ENTITY_TRAIL.invoke(engineHandle, entityId, maxPoints);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to set entity trail", e);
        }
    }
    
    /**
     * Set the camera position and target using the legacy API.
     * 
     * <p>This is the legacy World camera API that sets a single default camera.
     * For multi-viewport applications, use the viewport-specific camera API instead.</p>
     * 
     * <p><b>Thread Safety:</b> Must be called from the owning thread only.</p>
     * 
     * @param eyeX Camera position X
     * @param eyeY Camera position Y
     * @param eyeZ Camera position Z
     * @param targetX Look-at target X
     * @param targetY Look-at target Y
     * @param targetZ Look-at target Z
     * @param upX Up vector X (usually 0)
     * @param upY Up vector Y (usually 1)
     * @param upZ Up vector Z (usually 0)
     */
    public void setCamera(float eyeX, float eyeY, float eyeZ,
                         float targetX, float targetY, float targetZ,
                         float upX, float upY, float upZ) {
        checkClosed();
        try {
            EngineBindings.SET_CAMERA.invoke(engineHandle,
                eyeX, eyeY, eyeZ,
                targetX, targetY, targetZ,
                upX, upY, upZ);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to set camera", e);
        }
    }
    
    /**
     * Set the camera projection parameters using the legacy API.
     * 
     * <p>This sets the projection matrix for the default World camera.
     * For multi-viewport applications, use the viewport-specific camera API instead.</p>
     * 
     * <p><b>Thread Safety:</b> Must be called from the owning thread only.</p>
     * 
     * @param fovDegrees Field of view in degrees
     * @param nearPlane Near clipping plane distance
     * @param farPlane Far clipping plane distance
     */
    public void setCameraProjection(float fovDegrees, float nearPlane, float farPlane) {
        checkClosed();
        try {
            EngineBindings.SET_CAMERA_PROJECTION.invoke(engineHandle,
                fovDegrees, nearPlane, farPlane);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to set camera projection", e);
        }
    }
    
    /**
     * Check if engine is valid.
     * @return true if valid
     */
    public boolean isValid() {
        if (closed) {
            return false;
        }
        try {
            return (boolean) EngineBindings.IS_VALID.invoke(engineHandle);
        } catch (Throwable e) {
            return false;
        }
    }
    
    /**
     * Get the API version number.
     * Format: (MAJOR << 16) | (MINOR << 8) | PATCH
     */
    public int getApiVersion() {
        try {
            return (int) EngineBindings.API_VERSION.invoke();
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get API version", e);
        }
    }
    
    /**
     * Get the last error message from the engine.
     */
    public String getLastError() {
        try {
            MemorySegment errorPtr = (MemorySegment) EngineBindings.LAST_ERROR.invoke(engineHandle);
            if (errorPtr == null || errorPtr.equals(MemorySegment.NULL)) {
                return null;
            }
            return errorPtr.reinterpret(Long.MAX_VALUE).getString(0);
        } catch (Throwable e) {
            return "Failed to retrieve error: " + e.getMessage();
        }
    }
    
    /**
     * Create a viewport for rendering.
     * @param width Viewport width
     * @param height Viewport height
     */
    public NativeViewport createViewport(int width, int height) {
        checkClosed();
        try {
            // Allocate ViewportConfig
            MemorySegment config = arena.allocate(EngineBindings.VIEWPORT_CONFIG_LAYOUT);
            
            VarHandle widthHandle = EngineBindings.VIEWPORT_CONFIG_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("width"));
            VarHandle heightHandle = EngineBindings.VIEWPORT_CONFIG_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("height"));
            VarHandle aspectHandle = EngineBindings.VIEWPORT_CONFIG_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("aspect_ratio"));
            
            widthHandle.set(config, 0L, width);
            heightHandle.set(config, 0L, height);
            aspectHandle.set(config, 0L, (float) width / (float) height);
            
            // Allocate output pointer for viewport handle
            MemorySegment viewportHandlePtr = arena.allocate(ValueLayout.ADDRESS);
            
            int result = (int) EngineBindings.VIEWPORT_CREATE.invoke(
                engineHandle, config, viewportHandlePtr);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                String error = getLastError();
                throw new RuntimeException("Failed to create viewport, error code: " + result + 
                                         (error != null ? ", message: " + error : ""));
            }
            
            MemorySegment viewportHandle = viewportHandlePtr.get(ValueLayout.ADDRESS, 0L);
            return new NativeViewport(viewportHandle, this, arena);
            
        } catch (Throwable e) {
            throw new RuntimeException("Failed to create viewport", e);
        }
    }
    
    /**
     * Create a material.
     * @param desc Material descriptor
     */
    public NativeMaterial createMaterial(NativeMaterial.MaterialDesc desc) {
        checkClosed();
        try {
            // Allocate MaterialDesc
            MemorySegment descSegment = arena.allocate(EngineBindings.MATERIAL_DESC_LAYOUT);
            
            VarHandle base_r_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("base_color_r"));
            VarHandle base_g_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("base_color_g"));
            VarHandle base_b_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("base_color_b"));
            VarHandle base_a_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("base_color_a"));
            VarHandle metallic_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("metallic"));
            VarHandle roughness_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("roughness"));
            VarHandle alpha_mode_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("alpha_mode"));
            VarHandle base_tex_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("base_color_texture_id"));
            VarHandle normal_tex_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("normal_texture_id"));
            
            base_r_handle.set(descSegment, 0L, desc.baseColorR());
            base_g_handle.set(descSegment, 0L, desc.baseColorG());
            base_b_handle.set(descSegment, 0L, desc.baseColorB());
            base_a_handle.set(descSegment, 0L, desc.baseColorA());
            metallic_handle.set(descSegment, 0L, desc.metallic());
            roughness_handle.set(descSegment, 0L, desc.roughness());
            alpha_mode_handle.set(descSegment, 0L, desc.alphaMode());
            base_tex_handle.set(descSegment, 0L, desc.baseColorTextureId());
            normal_tex_handle.set(descSegment, 0L, desc.normalTextureId());
            
            // Allocate output pointer for material handle
            MemorySegment materialHandlePtr = arena.allocate(ValueLayout.ADDRESS);
            
            int result = (int) EngineBindings.MATERIAL_CREATE.invoke(
                engineHandle, descSegment, materialHandlePtr);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                String error = getLastError();
                throw new RuntimeException("Failed to create material, error code: " + result +
                                         (error != null ? ", message: " + error : ""));
            }
            
            MemorySegment materialHandle = materialHandlePtr.get(ValueLayout.ADDRESS, 0L);
            return new NativeMaterial(materialHandle, arena);
            
        } catch (Throwable e) {
            throw new RuntimeException("Failed to create material", e);
        }
    }
    
    private void checkClosed() {
        if (closed) {
            throw new IllegalStateException("Engine has been closed");
        }
    }
    
    @Override
    public void close() {
        if (closed) {
            return;
        }
        
        try {
            if (engineHandle != null && !engineHandle.equals(MemorySegment.NULL)) {
                EngineBindings.DESTROY_ENGINE.invoke(engineHandle);
            }
        } catch (Throwable e) {
            System.err.println("Error destroying engine: " + e.getMessage());
        } finally {
            arena.close();
            closed = true;
            System.out.println("[NativeEngine] Engine closed");
        }
    }
}
