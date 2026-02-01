package com.astraeus.rendering;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.model.PickResult;
import javafx.animation.AnimationTimer;
import javafx.scene.layout.Region;
import javafx.scene.layout.StackPane;

import java.util.function.Consumer;

/**
 * ViewportPane - Main container for 3D viewport with lifecycle management.
 * 
 * This is a JavaFX Region that owns the viewport UI surface and manages:
 * - Frame presentation via AnimationTimer
 * - Resize propagation with debouncing
 * - Lifecycle (init, update, cleanup)
 * 
 * USAGE:
 * <pre>
 * ViewportPane viewport = new ViewportPane(engine, 2560, 1440);
 * viewport.setOnEntitySelected(pick -> handleSelection(pick));
 * // Add to scene graph
 * centerPane.getChildren().add(viewport);
 * </pre>
 */
public class ViewportPane extends Region {
    
    private final NativeEngine engine;
    private final FxViewportSurface surface;
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
    
    /**
     * Create a new ViewportPane with default initial size.
     * 
     * @param engine Native engine instance
     * @param maxWidth Maximum backing buffer width
     * @param maxHeight Maximum backing buffer height
     */
    public ViewportPane(NativeEngine engine, int maxWidth, int maxHeight) {
        this(engine, maxWidth, maxHeight, 1280, 720);
    }
    
    /**
     * Create a new ViewportPane with specified initial size.
     * 
     * @param engine Native engine instance
     * @param maxWidth Maximum backing buffer width
     * @param maxHeight Maximum backing buffer height
     * @param initialWidth Initial viewport width
     * @param initialHeight Initial viewport height
     */
    public ViewportPane(NativeEngine engine, int maxWidth, int maxHeight,
                        int initialWidth, int initialHeight) {
        this.engine = engine;
        
        // Create viewport surface
        this.surface = new FxViewportSurface(engine, maxWidth, maxHeight, 
                                            initialWidth, initialHeight);
        
        // Add surface to this pane
        getChildren().add(surface);
        
        // Bind surface size to this pane's size
        surface.prefWidthProperty().bind(widthProperty());
        surface.prefHeightProperty().bind(heightProperty());
        surface.minWidthProperty().bind(widthProperty());
        surface.minHeightProperty().bind(heightProperty());
        surface.maxWidthProperty().bind(widthProperty());
        surface.maxHeightProperty().bind(heightProperty());
        
        // Setup resize listener with debouncing
        widthProperty().addListener((obs, oldVal, newVal) -> scheduleResize());
        heightProperty().addListener((obs, oldVal, newVal) -> scheduleResize());
        
        // Create render timer
        renderTimer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                if (!running) {
                    return;
                }
                
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
                surface.update(deltaTime);
                
                // Sync camera state to engine (legacy API)
                syncCameraToEngine();
                
                // Render frame
                try {
                    engine.beginFrame(deltaTime);
                    engine.endFrame();
                    
                    // Update display (triggers JavaFX to redraw)
                    surface.updateDisplay();
                    
                } catch (Exception e) {
                    System.err.println("[ViewportPane] Render error: " + e.getMessage());
                }
            }
        };
        
        System.out.println("[ViewportPane] Created with max=" + maxWidth + "x" + maxHeight);
    }
    
    /**
     * Start the render loop.
     * Call this when the viewport becomes visible.
     */
    public void start() {
        if (!running) {
            running = true;
            lastFrameTime = 0; // Reset timing
            renderTimer.start();
            System.out.println("[ViewportPane] Render loop started");
        }
    }
    
    /**
     * Stop the render loop.
     * Call this when the viewport becomes hidden or is being disposed.
     */
    public void stop() {
        if (running) {
            running = false;
            renderTimer.stop();
            System.out.println("[ViewportPane] Render loop stopped");
        }
    }
    
    /**
     * Check if render loop is running.
     */
    public boolean isRunning() {
        return running;
    }
    
    /**
     * Schedule a resize operation (debounced).
     */
    private void scheduleResize() {
        double w = getWidth();
        double h = getHeight();
        
        // Ignore invalid sizes
        if (w <= 0 || h <= 0) {
            return;
        }
        
        // Convert to device pixels (account for scale factor)
        // For now, assume 1:1 scaling. In production, you'd query the screen scale.
        int deviceW = (int) Math.round(w);
        int deviceH = (int) Math.round(h);
        
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
                surface.resizeViewport(pendingWidth, pendingHeight);
                
                // Clear pending
                pendingWidth = -1;
                pendingHeight = -1;
            }
        }
    }
    
    /**
     * Set entity selection callback.
     */
    public void setOnEntitySelected(Consumer<PickResult> callback) {
        surface.setOnEntitySelected(callback);
    }
    
    /**
     * Get the viewport controller for camera manipulation.
     */
    public ViewportController getController() {
        return surface.getController();
    }
    
    /**
     * Get the underlying surface.
     */
    public FxViewportSurface getSurface() {
        return surface;
    }
    
    /**
     * Get the native engine.
     */
    public NativeEngine getEngine() {
        return engine;
    }
    
    /**
     * Clean up resources.
     * Call this when the viewport is being disposed.
     */
    public void dispose() {
        stop();
        System.out.println("[ViewportPane] Disposed");
    }
    
    /**
     * Sync camera state from controller to engine.
     * Called every frame before rendering.
     */
    private void syncCameraToEngine() {
        try {
            ViewportController controller = surface.getController();
            double[] pos = controller.getCameraPosition();
            double[] target = controller.getCameraTarget();
            
            // Update engine camera using legacy API
            engine.setCamera(
                (float) pos[0], (float) pos[1], (float) pos[2],
                (float) target[0], (float) target[1], (float) target[2],
                0.0f, 1.0f, 0.0f  // Up vector (Y-up)
            );
        } catch (Exception e) {
            System.err.println("[ViewportPane] Camera sync error: " + e.getMessage());
        }
    }
}
