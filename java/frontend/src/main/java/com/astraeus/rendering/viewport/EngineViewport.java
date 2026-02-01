package com.astraeus.rendering.viewport;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.NativeViewport;
import com.astraeus.native_api.model.PixelBufferView;
import com.astraeus.native_api.model.PickResult;
import com.astraeus.rendering.ViewportController;
import com.astraeus.rendering.OverlayStack;
import com.astraeus.rendering.buffers.PixelBufferManager;
import javafx.geometry.Pos;
import javafx.scene.image.ImageView;
import javafx.scene.image.WritableImage;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.ScrollEvent;
import javafx.scene.layout.StackPane;
import javafx.scene.shape.Rectangle;
import javafx.scene.paint.Color;
import javafx.scene.control.Label;
import javafx.geometry.Insets;

import java.nio.ByteBuffer;
import java.util.function.Consumer;

/**
 * EngineViewport - Complete viewport component integrating native engine with JavaFX.
 * 
 * <p>This component provides a full-featured 3D viewport with:</p>
 * <ul>
 *   <li>Stable PixelBuffer integration with native engine color buffer</li>
 *   <li>DPI-aware coordinate conversion for HiDPI displays</li>
 *   <li>Resize handling without transient reallocations</li>
 *   <li>ID buffer support for entity picking</li>
 *   <li>Camera control via ViewportController</li>
 *   <li>Overlay stack for HUD elements</li>
 * </ul>
 * 
 * <p><b>Lifecycle:</b></p>
 * <ol>
 *   <li>Create EngineViewport with engine and max dimensions</li>
 *   <li>EngineViewport creates NativeViewport internally</li>
 *   <li>Call update(deltaTime) each frame before rendering</li>
 *   <li>Engine renders to viewport</li>
 *   <li>Call updateDisplay() after frame to update JavaFX</li>
 *   <li>Call close() when done to release native resources</li>
 * </ol>
 * 
 * <p><b>Thread Safety:</b> This class is NOT thread-safe. All methods must be
 * called from the JavaFX Application Thread.</p>
 * 
 * <p><b>Memory Safety:</b> The backing PixelBuffer is created once with maximum
 * dimensions and never reallocated. Resizes only change the viewport region,
 * not the backing buffer size.</p>
 * 
 * @see com.astraeus.native_api.NativeViewport
 * @see com.astraeus.rendering.buffers.PixelBufferManager
 */
public class EngineViewport extends StackPane implements AutoCloseable {
    
    // Core components
    private final NativeEngine engine;
    private final NativeViewport nativeViewport;
    private final PixelBufferManager bufferManager;
    private final ViewportController controller;
    private final OverlayStack overlayStack;
    
    // Image display
    private final ImageView imageView;
    private WritableImage writableImage;
    
    // Viewport dimensions
    private final int maxWidth;
    private final int maxHeight;
    private int currentWidth;
    private int currentHeight;
    
    // Camera projection state
    private float currentFovDegrees = 60.0f;
    private float currentNearPlane = 0.1f;
    private float currentFarPlane = 1000.0f;
    
    // Selection state
    private int selectedEntityId = 0;
    private Rectangle selectionRect;
    private Consumer<PickResult> onEntitySelected;
    
    // HUD overlays
    private Label cameraInfoLabel;
    
    // Input state flags
    private boolean inputEnabled = true;
    private boolean pickingEnabled = true;
    
    /**
     * Create a new EngineViewport.
     * 
     * @param engine Native engine instance
     * @param maxWidth Maximum viewport width in device pixels
     * @param maxHeight Maximum viewport height in device pixels
     * @param initialWidth Initial viewport width in device pixels
     * @param initialHeight Initial viewport height in device pixels
     * @throws RuntimeException if viewport creation fails
     */
    public EngineViewport(NativeEngine engine, int maxWidth, int maxHeight,
                         int initialWidth, int initialHeight) {
        this.engine = engine;
        this.maxWidth = maxWidth;
        this.maxHeight = maxHeight;
        this.currentWidth = initialWidth;
        this.currentHeight = initialHeight;
        
        // Create native viewport
        this.nativeViewport = engine.createViewport(maxWidth, maxHeight);
        
        // Resize to initial dimensions with projection
        nativeViewport.resizeWithProjection(initialWidth, initialHeight, 
                                           currentFovDegrees, currentNearPlane, currentFarPlane);
        
        // Create buffer manager
        this.bufferManager = new PixelBufferManager(maxWidth, maxHeight);
        
        // Get color buffer and initialize pixel buffer
        PixelBufferView colorView = nativeViewport.getColorBuffer();
        bufferManager.initialize(colorView);
        
        // Create WritableImage with PixelBuffer
        writableImage = new WritableImage(bufferManager.getPixelBuffer());
        
        // Create image view
        imageView = new ImageView(writableImage);
        imageView.setPreserveRatio(false);
        imageView.setSmooth(false);
        imageView.setViewport(new javafx.geometry.Rectangle2D(0, 0, initialWidth, initialHeight));
        imageView.fitWidthProperty().bind(widthProperty());
        imageView.fitHeightProperty().bind(heightProperty());
        
        // Create controller (pass native viewport's camera)
        this.controller = new ViewportController(engine);
        
        // Create overlay stack
        this.overlayStack = new OverlayStack();
        
        // Setup overlays
        createSelectionOverlay();
        createCameraInfoOverlay();
        
        // Add to stack pane (bottom to top)
        getChildren().addAll(imageView, overlayStack);
        
        // Setup input handlers
        setupInputHandlers();
        
        // Request focus for keyboard input
        setFocusTraversable(true);
        
        System.out.println("[EngineViewport] Created viewport " + initialWidth + "x" + initialHeight + 
                          " (max " + maxWidth + "x" + maxHeight + ")");
    }
    
    /**
     * Create selection rectangle overlay.
     */
    private void createSelectionOverlay() {
        selectionRect = new Rectangle();
        selectionRect.setFill(Color.TRANSPARENT);
        selectionRect.setStroke(Color.YELLOW);
        selectionRect.setStrokeWidth(3);
        selectionRect.setVisible(false);
        selectionRect.setMouseTransparent(true);
        
        overlayStack.addOverlay("selection", selectionRect, OverlayStack.Layer.SELECTION);
    }
    
    /**
     * Create camera info overlay.
     */
    private void createCameraInfoOverlay() {
        cameraInfoLabel = new Label();
        cameraInfoLabel.setStyle(
            "-fx-background-color: rgba(0, 0, 0, 0.6); " +
            "-fx-text-fill: white; " +
            "-fx-padding: 5px; " +
            "-fx-font-size: 11px; " +
            "-fx-font-family: 'Consolas', 'Courier New', monospace;"
        );
        cameraInfoLabel.setMouseTransparent(true);
        cameraInfoLabel.setVisible(false);
        
        overlayStack.addOverlay("camera-info", cameraInfoLabel, 
                               OverlayStack.Layer.HUD, Pos.BOTTOM_LEFT);
        StackPane.setMargin(cameraInfoLabel, new Insets(10));
    }
    
    /**
     * Setup input event handlers.
     */
    private void setupInputHandlers() {
        // Mouse handlers for camera control
        setOnMousePressed(event -> {
            requestFocus();
            if (inputEnabled && !event.isConsumed()) {
                controller.handleMousePressed(event);
                event.consume();
            }
        });
        
        setOnMouseDragged(event -> {
            if (inputEnabled && !event.isConsumed()) {
                controller.handleMouseDragged(event);
                event.consume();
            }
        });
        
        setOnMouseReleased(event -> {
            if (inputEnabled && !event.isConsumed()) {
                controller.handleMouseReleased(event);
                event.consume();
            }
        });
        
        // Mouse click for picking
        setOnMouseClicked(event -> {
            if (pickingEnabled && !event.isConsumed()) {
                handleMouseClick(event);
                event.consume();
            }
        });
        
        // Scroll for zoom
        setOnScroll(event -> {
            if (inputEnabled && !event.isConsumed()) {
                controller.handleScroll(event);
                event.consume();
            }
        });
        
        // Keyboard handlers
        setOnKeyPressed(event -> {
            if (inputEnabled && !event.isConsumed()) {
                handleKeyPressed(event);
                event.consume();
            }
        });
        
        setOnKeyReleased(event -> {
            if (inputEnabled && !event.isConsumed()) {
                controller.handleKeyReleased(event);
                event.consume();
            }
        });
    }
    
    /**
     * Handle mouse click for entity picking.
     */
    private void handleMouseClick(MouseEvent event) {
        try {
            // Convert scene coordinates to viewport coordinates
            double sceneX = event.getX();
            double sceneY = event.getY();
            
            // Calculate scale factors (account for image view scaling)
            double scaleX = currentWidth / imageView.getFitWidth();
            double scaleY = currentHeight / imageView.getFitHeight();
            
            // Convert to viewport pixels (device coordinates)
            int viewportX = (int) (sceneX * scaleX);
            int viewportY = (int) (sceneY * scaleY);
            
            // Clamp to bounds
            viewportX = Math.max(0, Math.min(viewportX, currentWidth - 1));
            viewportY = Math.max(0, Math.min(viewportY, currentHeight - 1));
            
            // Perform pick
            PickResult result = engine.pick(viewportX, viewportY);
            
            // Update selection
            if (result.hasValidEntity()) {
                selectedEntityId = result.getEntityId();
                updateSelectionOverlay(sceneX, sceneY);
                
                if (onEntitySelected != null) {
                    onEntitySelected.accept(result);
                }
            } else {
                clearSelection();
                
                if (onEntitySelected != null) {
                    onEntitySelected.accept(result);
                }
            }
            
        } catch (Exception e) {
            System.err.println("[EngineViewport] Picking error: " + e.getMessage());
        }
    }
    
    /**
     * Handle key pressed events.
     */
    private void handleKeyPressed(KeyEvent event) {
        switch (event.getCode()) {
            case F1:
                // Toggle camera info
                overlayStack.toggleOverlay("camera-info");
                break;
            case ESCAPE:
                clearSelection();
                break;
            default:
                // Pass to controller
                controller.handleKeyPressed(event);
                break;
        }
    }
    
    /**
     * Update viewport state.
     * Call this every frame before rendering.
     * 
     * @param deltaTime Time since last update (seconds)
     */
    public void update(double deltaTime) {
        // Update camera controller
        controller.update(deltaTime);
        
        // Update camera info overlay if visible
        if (overlayStack.isOverlayVisible("camera-info")) {
            updateCameraInfo();
        }
    }
    
    /**
     * Update display with latest engine output.
     * Call this after rendering each frame.
     */
    public void updateDisplay() {
        // Update the buffer manager (triggers JavaFX to redraw)
        bufferManager.updateBuffer();
    }
    
    /**
     * Resize viewport.
     * 
     * <p>This changes the viewport region without reallocating the backing buffer.
     * The backing buffer remains at the maximum size specified during construction.</p>
     * 
     * @param width New viewport width in device pixels
     * @param height New viewport height in device pixels
     */
    public void resize(int width, int height) {
        width = Math.min(width, maxWidth);
        height = Math.min(height, maxHeight);
        
        if (width == currentWidth && height == currentHeight) {
            return;
        }
        
        // Use authoritative resize method that updates both viewport AND projection
        nativeViewport.resizeWithProjection(width, height, 
                                           currentFovDegrees, currentNearPlane, currentFarPlane);
        
        // Log frame info for debugging (VIS-003)
        System.out.println("[EngineViewport] FrameInfo after resize:");
        System.out.println("  Requested: " + width + "x" + height);
        System.out.println("  Aspect: " + ((float) width / (float) height));
        System.out.println("  Camera FOV: " + currentFovDegrees);
        
        // Update buffer manager
        bufferManager.updateViewportSize(width, height);
        
        // Update image view viewport region
        imageView.setViewport(new javafx.geometry.Rectangle2D(0, 0, width, height));
        
        currentWidth = width;
        currentHeight = height;
        
        System.out.println("[EngineViewport] Resized to " + width + "x" + height);
    }
    
    /**
     * Update camera info display.
     */
    private void updateCameraInfo() {
        String info = controller.getDebugInfo();
        cameraInfoLabel.setText(info);
    }
    
    /**
     * Update selection overlay.
     */
    private void updateSelectionOverlay(double centerX, double centerY) {
        double size = 40;
        selectionRect.setX(centerX - size / 2);
        selectionRect.setY(centerY - size / 2);
        selectionRect.setWidth(size);
        selectionRect.setHeight(size);
        selectionRect.setVisible(true);
    }
    
    /**
     * Clear selection.
     */
    public void clearSelection() {
        selectedEntityId = 0;
        selectionRect.setVisible(false);
    }
    
    /**
     * Get viewport controller.
     */
    public ViewportController getController() {
        return controller;
    }
    
    /**
     * Get overlay stack.
     */
    public OverlayStack getOverlayStack() {
        return overlayStack;
    }
    
    /**
     * Get native viewport handle.
     */
    public NativeViewport getNativeViewport() {
        return nativeViewport;
    }
    
    /**
     * Get native engine reference.
     */
    public NativeEngine getEngine() {
        return engine;
    }
    
    /**
     * Get buffer manager.
     */
    public PixelBufferManager getBufferManager() {
        return bufferManager;
    }
    
    /**
     * Set entity selection callback.
     */
    public void setOnEntitySelected(Consumer<PickResult> callback) {
        this.onEntitySelected = callback;
    }
    
    /**
     * Get selected entity ID.
     */
    public int getSelectedEntityId() {
        return selectedEntityId;
    }
    
    /**
     * Enable/disable input handling.
     */
    public void setInputEnabled(boolean enabled) {
        this.inputEnabled = enabled;
    }
    
    /**
     * Check if input is enabled.
     */
    public boolean isInputEnabled() {
        return inputEnabled;
    }
    
    /**
     * Enable/disable picking.
     */
    public void setPickingEnabled(boolean enabled) {
        this.pickingEnabled = enabled;
    }
    
    /**
     * Get current viewport width.
     */
    public int getCurrentWidth() {
        return currentWidth;
    }
    
    /**
     * Get current viewport height.
     */
    public int getCurrentHeight() {
        return currentHeight;
    }
    
    /**
     * Get maximum viewport width.
     */
    public int getMaxViewportWidth() {
        return maxWidth;
    }
    
    /**
     * Get maximum viewport height.
     */
    public int getMaxViewportHeight() {
        return maxHeight;
    }
    
    /**
     * Close and release native resources.
     */
    @Override
    public void close() {
        nativeViewport.close();
        System.out.println("[EngineViewport] Closed");
    }
}
