package com.astraeus.native_api;

import java.lang.foreign.*;
import java.lang.invoke.VarHandle;

/**
 * High-level Java wrapper around the native Astraeus engine.
 * Manages native memory and provides a safe API.
 */
public class NativeEngine implements AutoCloseable {
    
    private MemorySegment engineHandle;
    private final Arena arena;
    private boolean closed = false;
    
    /**
     * Create a new engine instance.
     * @param width Initial viewport width
     * @param height Initial viewport height
     * @param enableValidation Enable validation layers
     */
    public NativeEngine(int width, int height, boolean enableValidation) {
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
        
        widthHandle.set(config, 0L, width);
        heightHandle.set(config, 0L, height);
        validationHandle.set(config, 0L, enableValidation);
        debugHandle.set(config, 0L, false);
        logPathHandle.set(config, 0L, MemorySegment.NULL);
        
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
     */
    public void beginFrame(double deltaTime) {
        checkClosed();
        try {
            EngineBindings.BEGIN_FRAME.invoke(engineHandle, deltaTime);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to begin frame", e);
        }
    }
    
    /**
     * End the current frame.
     */
    public void endFrame() {
        checkClosed();
        try {
            EngineBindings.END_FRAME.invoke(engineHandle);
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
     */
    public PixelBufferView getColorBuffer() {
        checkClosed();
        try {
            // Allocate memory for out-parameter
            MemorySegment viewStruct = arena.allocate(EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT);
            
            // Call native function with out-parameter
            EngineBindings.GET_COLOR_BUFFER.invoke(engineHandle, viewStruct);
            
            // Validate the result before creating PixelBufferView
            VarHandle dataHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("data"));
            MemorySegment dataPtr = (MemorySegment) dataHandle.get(viewStruct, 0L);
            
            if (dataPtr == null || dataPtr.equals(MemorySegment.NULL)) {
                throw new RuntimeException("Failed to get color buffer: invalid data pointer");
            }
            
            return new PixelBufferView(viewStruct);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get color buffer", e);
        }
    }
    
    /**
     * Get the ID buffer view for picking.
     * The returned PixelBufferView has a stable data pointer that never changes.
     * Only the viewport dimensions (width/height) change on resize.
     */
    public PixelBufferView getIdBuffer() {
        checkClosed();
        try {
            // Allocate memory for out-parameter
            MemorySegment viewStruct = arena.allocate(EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT);
            
            // Call native function with out-parameter
            EngineBindings.GET_ID_BUFFER.invoke(engineHandle, viewStruct);
            
            // Validate the result before creating PixelBufferView
            VarHandle dataHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("data"));
            MemorySegment dataPtr = (MemorySegment) dataHandle.get(viewStruct, 0L);
            
            if (dataPtr == null || dataPtr.equals(MemorySegment.NULL)) {
                throw new RuntimeException("Failed to get ID buffer: invalid data pointer");
            }
            
            return new PixelBufferView(viewStruct);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get ID buffer", e);
        }
    }
    
    /**
     * Perform picking at screen coordinates.
     * Returns information about the entity at the specified screen position.
     * 
     * @param screenX Screen X coordinate (0 = left edge)
     * @param screenY Screen Y coordinate (0 = top edge)
     * @return PickingView containing pick result data
     */
    public PickingView pick(int screenX, int screenY) {
        checkClosed();
        try {
            MemorySegment resultStruct = (MemorySegment) EngineBindings.PICK.invoke(
                engineHandle, screenX, screenY);
            return new PickingView(resultStruct);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to perform picking", e);
        }
    }
    
    /**
     * Wrapper for PixelBufferView struct.
     * Provides safe access to backing buffer without memory hazards.
     */
    public static class PixelBufferView {
        private final MemorySegment data;
        private final int width;
        private final int height;
        private final int stride;
        private final int format;
        private final int maxBackingWidth;
        private final int maxBackingHeight;
        private final int maxBackingSize;
        
        public PixelBufferView(MemorySegment structSegment) {
            VarHandle dataHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("data"));
            VarHandle widthHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("width"));
            VarHandle heightHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("height"));
            VarHandle strideHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("stride"));
            VarHandle formatHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("format"));
            VarHandle maxWidthHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("max_backing_width"));
            VarHandle maxHeightHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("max_backing_height"));
            VarHandle maxSizeHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("max_backing_size"));
            
            MemorySegment dataPtr = (MemorySegment) dataHandle.get(structSegment, 0L);
            this.width = (int) widthHandle.get(structSegment, 0L);
            this.height = (int) heightHandle.get(structSegment, 0L);
            this.stride = (int) strideHandle.get(structSegment, 0L);
            this.format = (int) formatHandle.get(structSegment, 0L);
            this.maxBackingWidth = (int) maxWidthHandle.get(structSegment, 0L);
            this.maxBackingHeight = (int) maxHeightHandle.get(structSegment, 0L);
            this.maxBackingSize = (int) maxSizeHandle.get(structSegment, 0L);
            
            // Reinterpret data pointer to access full backing buffer
            this.data = dataPtr.reinterpret(maxBackingSize);
        }
        
        public MemorySegment getData() { return data; }
        public int getWidth() { return width; }
        public int getHeight() { return height; }
        public int getStride() { return stride; }
        public int getFormat() { return format; }
        public int getMaxBackingWidth() { return maxBackingWidth; }
        public int getMaxBackingHeight() { return maxBackingHeight; }
        public int getMaxBackingSize() { return maxBackingSize; }
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
