package com.astraeus.rendering;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.model.FrameStats;
import com.astraeus.native_api.model.PickResult;
import com.astraeus.native_api.model.PixelBufferView;
import com.astraeus.tools.TelemetryOverlay;
import com.astraeus.ui.viewport.PickingCoordinateTransform;
import javafx.geometry.Pos;
import javafx.geometry.Rectangle2D;
import javafx.scene.image.ImageView;
import javafx.scene.image.PixelBuffer;
import javafx.scene.image.PixelFormat;
import javafx.scene.image.WritableImage;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.StackPane;
import javafx.scene.shape.Rectangle;
import javafx.scene.paint.Color;
import javafx.scene.control.Label;
import javafx.geometry.Insets;
import javafx.util.Callback;

import java.nio.ByteBuffer;
import java.util.function.Consumer;

/**
 * Enhanced JavaFX viewport component with input routing, camera control, and overlays.
 * 
 * FEATURES:
 * - Input routing for mouse and keyboard
 * - Integrated camera controller (orbit/fly/pan modes)
 * - Overlay stack for HUD, selection, gizmos
 * - Multiple independent viewport support
 * - No per-frame allocations
 * 
 * SAFETY GUARANTEES:
 * - PixelBuffer backing memory allocated once, never resized
 * - Viewport-only resize operations
 * - Stable memory pointer
 * 
 * USAGE:
 * <pre>
 * FxViewport viewport = new FxViewport(engine, 2560, 1440, 1280, 720);
 * viewport.getController().setMode(ViewportController.Mode.ORBIT);
 * viewport.setOnEntitySelected(result -> handlePick(result));
 * 
 * // In render loop:
 * viewport.update(deltaTime);
 * viewport.updateDisplay();
 * </pre>
 */
public class FxViewport extends StackPane {
    
    // Core components
    private final NativeEngine engine;
    private final ViewportController controller;
    private final OverlayStack overlayStack;
    private final PickingCoordinateTransform coordinateTransform;
    
    // Image display
    private final ImageView imageView;
    private WritableImage writableImage;
    private PixelBuffer<ByteBuffer> pixelBuffer;
    private PixelBufferView colorBuffer;
    
    // Viewport dimensions
    private final int maxWidth;
    private final int maxHeight;
    private int currentWidth;
    private int currentHeight;
    
    // Selection state
    private int selectedEntityId = 0;
    private Rectangle selectionRect;
    private Consumer<PickResult> onEntitySelected;
    
    // HUD overlays
    private Label cameraInfoLabel;
    private TelemetryOverlay telemetryOverlay;
    
    // Pre-allocated Rectangle2D for updateDisplay (zero allocations)
    private volatile Rectangle2D dirtyRect = null; // null => full buffer dirty
    private int lastDirtyW = -1, lastDirtyH = -1;
    private final Callback<PixelBuffer<ByteBuffer>, Rectangle2D> DIRTY_CB = pb -> dirtyRect;

    private boolean warnedBufferState;
    
    // Input state flags
    private boolean inputEnabled = true;
    private boolean pickingEnabled = true;

    // Debugging
    private static final boolean ASSERT_BUF_STATE = Boolean.getBoolean("astraeus.debug.assertBufferState");
    
    /**
     * Create a new enhanced viewport with camera control and overlays.
     * 
     * @param engine Native engine instance
     * @param maxWidth Maximum viewport width
     * @param maxHeight Maximum viewport height
     * @param initialWidth Initial viewport width
     * @param initialHeight Initial viewport height
     */
    public FxViewport(NativeEngine engine, int maxWidth, int maxHeight,
                        int initialWidth, int initialHeight) {
        this.engine = engine;
        this.maxWidth = maxWidth;
        this.maxHeight = maxHeight;
        this.currentWidth = initialWidth;
        this.currentHeight = initialHeight;
        
        // Create controller
        controller = new ViewportController(engine);

        // Configure readback
        engine.configureReadback(maxWidth, maxHeight, false);
        PixelBufferView colorView = engine.getColorBuffer();
        ByteBuffer backingBuffer = colorView.getByteBuffer();
        
        if (backingBuffer == null) {
            throw new IllegalStateException("Color ByteBuffer not attached");
        }
        
        // Create pixel buffer
        PixelFormat<ByteBuffer> format = PixelFormat.getByteBgraPreInstance();
        int backingW = colorView.getMaxBackingWidth();
        int backingH = colorView.getMaxBackingHeight();
        
        pixelBuffer = new PixelBuffer<>(backingW, backingH, backingBuffer, format);
        writableImage = new WritableImage(pixelBuffer);
        
        // Create image view
        imageView = new ImageView(writableImage);
        imageView.setPreserveRatio(false);
        imageView.setSmooth(false);
        imageView.setViewport(new javafx.geometry.Rectangle2D(0, 0, initialWidth, initialHeight));
        imageView.fitWidthProperty().bind(widthProperty());
        imageView.fitHeightProperty().bind(heightProperty());
        
        // Create overlay stack
        overlayStack = new OverlayStack();

        // Create coordinate transform for picking
        coordinateTransform = new PickingCoordinateTransform(imageView);
        coordinateTransform.setViewportDimensions(initialWidth, initialHeight);
        
        // Setup overlays
        createSelectionOverlay();
        createCameraInfoOverlay();
        createTelemetryOverlay();
        
        // Add to stack pane (bottom to top)
        getChildren().addAll(imageView, overlayStack);
        
        // Setup input handlers
        setupInputHandlers();
        
        // Request focus for keyboard input
        setFocusTraversable(true);
        
        System.out.println("[FxViewport] Created viewport with controller and overlays");
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
     * Create telemetry overlay.
     */
    private void createTelemetryOverlay() {
        telemetryOverlay = new TelemetryOverlay();
        telemetryOverlay.setVisible(false);
        
        overlayStack.addOverlay("telemetry", telemetryOverlay,
                               OverlayStack.Layer.HUD, Pos.TOP_RIGHT);
        StackPane.setMargin(telemetryOverlay, new Insets(10));
    }
    
    /**
     * Setup input event handlers.
     */
    private void setupInputHandlers() {
        // Mouse handlers for camera control
        setOnMousePressed(event -> {
            requestFocus(); // Ensure we receive keyboard events
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
            // Convert scene coordinates to viewport coordinates using robust transform
            double sceneX = event.getX();
            double sceneY = event.getY();
            
            // Check if coordinates are within bounds
            if (!coordinateTransform.isWithinBounds(sceneX, sceneY)) {
                System.err.println("[FxViewport] Click outside viewport bounds");
                return;
            }
            
            // Transform to viewport pixel coordinates
            int[] viewportCoords = coordinateTransform.sceneToViewport(sceneX, sceneY);
            int viewportX = viewportCoords[0];
            int viewportY = viewportCoords[1];
            
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
            System.err.println("[FxViewport] Picking error: " + e.getMessage());
            e.printStackTrace();
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
            case F2:
                // Toggle telemetry
                overlayStack.toggleOverlay("telemetry");
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
        
        // Sync camera state to native engine (if available)
        // Note: This requires viewport-specific camera access, which may not be
        // available in the current MVP. For now, we'll update camera through
        // the legacy World API in the engine's render loop.
        
        // Update camera info overlay if visible
        if (overlayStack.isOverlayVisible("camera-info")) {
            updateCameraInfo();
        }
        
        // Update telemetry overlay if visible and enabled
        if (overlayStack.isOverlayVisible("telemetry") && engine.isTelemetryEnabled()) {
            try {
                FrameStats stats = engine.getTelemetryStats();
                telemetryOverlay.update(stats);
            } catch (Exception e) {
                // Log error but don't disrupt render loop
                System.err.println("[FxViewport] Telemetry update error: " + e.getMessage());
            }
        }
    }
    
    /**
     * Update display with latest engine output.
     * Call this after rendering each frame.
     * 
     * NOTE: Uses pre-allocated Rectangle2D to avoid per-frame allocations.
     */
    public void updateDisplay() {
        if (pixelBuffer == null) return;

        // Optional dev assert — but do NOT run unconditionally per frame.
        if (ASSERT_BUF_STATE && colorBuffer != null) {
            ByteBuffer b = colorBuffer.getByteBuffer();
            if (b != null && (b.position() != 0 || b.limit() != b.capacity())) {
                if (!warnedBufferState) {
                    warnedBufferState = true;
                    System.err.println("[FxViewport] WARNING: ByteBuffer state corrupted! " +
                            "pos=" + b.position() + " lim=" + b.limit() + " cap=" + b.capacity() +
                            " (repairing with clear())");
                }
                b.clear();
            }
        }

        // Clamp dirty region to PixelBuffer content bounds to avoid Prism crash.
        final int bufW = pixelBuffer.getWidth();
        final int bufH = pixelBuffer.getHeight();

        final int w = Math.max(0, Math.min(currentWidth,  bufW));
        final int h = Math.max(0, Math.min(currentHeight, bufH));

        // Avoid empty rectangles (can also throw in Prism); fall back to full dirty.
        // Also avoid per-frame Rectangle2D allocations: only allocate on size change.
        if (w <= 0 || h <= 0) {
            dirtyRect = null; // full buffer dirty
            lastDirtyW = lastDirtyH = -1;
        } else if (w == bufW && h == bufH) {
            dirtyRect = null; // full buffer dirty (and zero alloc) :contentReference[oaicite:0]{index=0}
            lastDirtyW = bufW;
            lastDirtyH = bufH;
        } else if (w != lastDirtyW || h != lastDirtyH) {
            dirtyRect = new Rectangle2D(0, 0, w, h); // allocate ONLY when size changes
            lastDirtyW = w;
            lastDirtyH = h;
        }

        // No per-frame allocations here
        pixelBuffer.updateBuffer(DIRTY_CB);
    }

    /**
     * Resize viewport.
     */
    public void resizeViewport(int width, int height) {
        width = Math.min(width, maxWidth);
        height = Math.min(height, maxHeight);
        
        if (width == currentWidth && height == currentHeight) {
            return;
        }
        
        engine.resizeViewport(width, height);
        imageView.setViewport(new javafx.geometry.Rectangle2D(0, 0, width, height));
        
        currentWidth = width;
        currentHeight = height;
        
        // Update coordinate transform for picking
        coordinateTransform.setViewportDimensions(width, height);
        
        colorBuffer = engine.getColorBuffer();
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
     * Get native engine reference.
     */
    public NativeEngine getEngine() {
        return engine;
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
    public int getViewportMaxWidth() {
        return maxWidth;
    }
    
    /**
     * Get maximum viewport height.
     */
    public int getViewportMaxHeight() {
        return maxHeight;
    }
}
