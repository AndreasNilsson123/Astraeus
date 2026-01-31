package com.astraeus.ui.viewport;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.model.PickResult;
import com.astraeus.rendering.viewport.EngineViewport;
import javafx.animation.AnimationTimer;
import javafx.scene.layout.Region;

import java.util.function.Consumer;

/**
 * ViewportPaneV2 - Container for EngineViewport with lifecycle and render loop management.
 * 
 * <p>This pane manages the complete viewport lifecycle including:</p>
 * <ul>
 *   <li>Viewport creation and destruction</li>
 *   <li>AnimationTimer-based render loop</li>
 *   <li>Debounced resize propagation to native viewport</li>
 *   <li>Frame timing and delta time calculation</li>
 * </ul>
 * 
 * <p><b>Lifecycle:</b></p>
 * <ol>
 *   <li>Create ViewportPaneV2 with engine and dimensions</li>
 *   <li>Call start() to begin render loop</li>
 *   <li>Render loop automatically calls engine.beginFrame/endFrame</li>
 *   <li>Call stop() to pause render loop</li>
 *   <li>Call dispose() when done to release all resources</li>
 * </ol>
 * 
 * <p><b>Resize Handling:</b> Resize events are debounced (100ms) to avoid
 * excessive native calls during live window resizing.</p>
 * 
 * <p><b>Thread Safety:</b> This class is NOT thread-safe. All methods must be
 * called from the JavaFX Application Thread.</p>
 * 
 * <p><b>Multi-Viewport Support:</b> Multiple ViewportPaneV2 instances can be
 * created. Each has its own NativeViewport and render loop. Only call
 * engine.beginFrame/endFrame from ONE active viewport at a time.</p>
 * 
 * @see com.astraeus.rendering.viewport.EngineViewport
 */
public class ViewportPaneV2 extends Region implements AutoCloseable {
    
    private final NativeEngine engine;
    private final EngineViewport viewport;
    private final AnimationTimer renderTimer;
    
    // Resize debouncing
    private long lastResizeTime = 0;
    private static final long RESIZE_DEBOUNCE_NS = 100_000_000; // 100ms
    private int pendingWidth = -1;
    private int pendingHeight = -1;
    
    // Frame timing
    private long lastFrameTime = 0;
    
    // Lifecycle
    private boolean running = false;
    private boolean disposed = false;
    
    /**
     * Create a new ViewportPaneV2 with default initial size.
     * 
     * @param engine Native engine instance
     * @param maxWidth Maximum viewport width in device pixels
     * @param maxHeight Maximum viewport height in device pixels
     */
    public ViewportPaneV2(NativeEngine engine, int maxWidth, int maxHeight) {
        this(engine, maxWidth, maxHeight, 1280, 720);
    }
    
    /**
     * Create a new ViewportPaneV2 with specified initial size.
     * 
     * @param engine Native engine instance
     * @param maxWidth Maximum viewport width in device pixels
     * @param maxHeight Maximum viewport height in device pixels
     * @param initialWidth Initial viewport width in device pixels
     * @param initialHeight Initial viewport height in device pixels
     */
    public ViewportPaneV2(NativeEngine engine, int maxWidth, int maxHeight,
                         int initialWidth, int initialHeight) {
        if (engine == null) {
            throw new NullPointerException("engine cannot be null");
        }
        
        this.engine = engine;
        
        // Create viewport
        this.viewport = new EngineViewport(engine, maxWidth, maxHeight,
                                          initialWidth, initialHeight);
        
        // Add viewport to this pane
        getChildren().add(viewport);
        
        // Bind viewport size to this pane's size
        viewport.prefWidthProperty().bind(widthProperty());
        viewport.prefHeightProperty().bind(heightProperty());
        viewport.minWidthProperty().bind(widthProperty());
        viewport.minHeightProperty().bind(heightProperty());
        viewport.maxWidthProperty().bind(widthProperty());
        viewport.maxHeightProperty().bind(heightProperty());
        
        // Setup resize listener with debouncing
        widthProperty().addListener((obs, oldVal, newVal) -> scheduleResize());
        heightProperty().addListener((obs, oldVal, newVal) -> scheduleResize());
        
        // Create render timer
        renderTimer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                if (!running || disposed) {
                    return;
                }
                
                try {
                    // Calculate delta time
                    double deltaTime = 0.016; // Default ~60 FPS
                    if (lastFrameTime > 0) {
                        deltaTime = (now - lastFrameTime) / 1_000_000_000.0;
                        deltaTime = Math.min(deltaTime, 0.1); // Cap at 100ms
                    }
                    lastFrameTime = now;
                    
                    // Check for pending resize
                    checkPendingResize(now);
                    
                    // Update viewport (camera, overlays, etc.)
                    viewport.update(deltaTime);
                    
                    // Render frame
                    engine.beginFrame(deltaTime);
                    engine.endFrame();
                    
                    // Update display (triggers JavaFX to redraw)
                    viewport.updateDisplay();
                    
                } catch (Exception e) {
                    System.err.println("[ViewportPaneV2] Render error: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        };
        
        System.out.println("[ViewportPaneV2] Created with max=" + maxWidth + "x" + maxHeight +
                          ", initial=" + initialWidth + "x" + initialHeight);
    }
    
    /**
     * Start the render loop.
     * Call this when the viewport becomes visible or active.
     */
    public void start() {
        if (disposed) {
            throw new IllegalStateException("ViewportPaneV2 has been disposed");
        }
        
        if (!running) {
            running = true;
            lastFrameTime = 0; // Reset timing
            renderTimer.start();
            System.out.println("[ViewportPaneV2] Render loop started");
        }
    }
    
    /**
     * Stop the render loop.
     * Call this when the viewport becomes hidden or inactive.
     */
    public void stop() {
        if (running) {
            running = false;
            renderTimer.stop();
            System.out.println("[ViewportPaneV2] Render loop stopped");
        }
    }
    
    /**
     * Check if render loop is running.
     * 
     * @return true if render loop is active
     */
    public boolean isRunning() {
        return running;
    }
    
    /**
     * Check if disposed.
     * 
     * @return true if disposed
     */
    public boolean isDisposed() {
        return disposed;
    }
    
    /**
     * Schedule a resize operation (debounced).
     */
    private void scheduleResize() {
        if (disposed) {
            return;
        }
        
        double w = getWidth();
        double h = getHeight();
        
        // Ignore invalid sizes
        if (w <= 0 || h <= 0) {
            return;
        }
        
        // Convert to device pixels (account for DPI scale factor)
        // For now, use viewport's buffer manager DPI scale
        // In production, query the screen scale factor
        double dpiScale = viewport.getBufferManager().getDpiScale();
        int deviceW = (int) Math.round(w * dpiScale);
        int deviceH = (int) Math.round(h * dpiScale);
        
        // Clamp to maximum
        deviceW = Math.min(deviceW, viewport.getMaxWidth());
        deviceH = Math.min(deviceH, viewport.getMaxHeight());
        
        // Store pending resize
        pendingWidth = deviceW;
        pendingHeight = deviceH;
        lastResizeTime = System.nanoTime();
    }
    
    /**
     * Check and apply pending resize if debounce period has elapsed.
     */
    private void checkPendingResize(long now) {
        if (pendingWidth > 0 && pendingHeight > 0) {
            long elapsed = now - lastResizeTime;
            if (elapsed >= RESIZE_DEBOUNCE_NS) {
                // Apply resize
                viewport.resize(pendingWidth, pendingHeight);
                
                // Clear pending
                pendingWidth = -1;
                pendingHeight = -1;
            }
        }
    }
    
    /**
     * Set entity selection callback.
     * 
     * @param callback Callback to invoke when entity is selected/deselected
     */
    public void setOnEntitySelected(Consumer<PickResult> callback) {
        viewport.setOnEntitySelected(callback);
    }
    
    /**
     * Get the underlying EngineViewport.
     * 
     * @return EngineViewport instance
     */
    public EngineViewport getViewport() {
        return viewport;
    }
    
    /**
     * Get the native engine.
     * 
     * @return NativeEngine instance
     */
    public NativeEngine getEngine() {
        return engine;
    }
    
    /**
     * Dispose and release all resources.
     * 
     * <p>This stops the render loop, closes the viewport, and marks this pane
     * as disposed. After calling dispose(), the pane cannot be reused.</p>
     * 
     * @throws IllegalStateException if already disposed
     */
    public void dispose() {
        if (disposed) {
            throw new IllegalStateException("ViewportPaneV2 already disposed");
        }
        
        stop();
        viewport.close();
        disposed = true;
        System.out.println("[ViewportPaneV2] Disposed");
    }
    
    /**
     * Close and release resources (AutoCloseable).
     */
    @Override
    public void close() {
        if (!disposed) {
            dispose();
        }
    }
}
