package com.astraeus.rendering;

import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.ScrollEvent;

/**
 * ViewportController handles camera control for a viewport.
 * 
 * PERFORMANCE:
 * - NO per-frame allocations
 * - Reuses internal state variables
 * - Efficient input handling
 * 
 * FEATURES:
 * - Orbit mode: Rotate camera around target point
 * - Fly mode: Free camera movement (WASD + mouse look)
 * - Pan mode: 2D panning (drag to pan)
 * - Smooth transitions between modes
 * 
 * USAGE:
 * <pre>
 * ViewportController controller = new ViewportController(engine);
 * controller.setMode(ViewportController.Mode.ORBIT);
 * viewport.setOnMousePressed(controller::handleMousePressed);
 * viewport.setOnMouseDragged(controller::handleMouseDragged);
 * viewport.setOnMouseReleased(controller::handleMouseReleased);
 * viewport.setOnScroll(controller::handleScroll);
 * viewport.setOnKeyPressed(controller::handleKeyPressed);
 * viewport.setOnKeyReleased(controller::handleKeyReleased);
 * 
 * // In update loop:
 * controller.update(deltaTime);
 * </pre>
 */
public class ViewportController {
    
    /**
     * Camera control modes.
     */
    public enum Mode {
        /** Orbit camera around a target point */
        ORBIT,
        /** Free-flying camera (FPS-style) */
        FLY,
        /** 2D panning mode */
        PAN
    }
    
    // Current mode
    private Mode mode = Mode.ORBIT;
    
    // Mouse state (reused to avoid allocations)
    private boolean isDragging = false;
    private double lastMouseX = 0;
    private double lastMouseY = 0;
    private MouseButton dragButton = null;
    
    // Keyboard state (reused to avoid allocations)
    private boolean keyW = false;
    private boolean keyA = false;
    private boolean keyS = false;
    private boolean keyD = false;
    private boolean keyQ = false;
    private boolean keyE = false;
    private boolean keyShift = false;
    
    // Orbit mode state
    private double orbitDistance = 10.0;
    private double orbitAzimuth = 0.0;   // Horizontal rotation (radians)
    private double orbitElevation = 0.5; // Vertical rotation (radians)
    private double orbitTargetX = 0.0;
    private double orbitTargetY = 0.0;
    private double orbitTargetZ = 0.0;
    
    // Fly mode state
    private double flyCameraX = 0.0;
    private double flyCameraY = 5.0;
    private double flyCameraZ = -10.0;
    private double flyYaw = 0.0;         // Horizontal look (radians)
    private double flyPitch = 0.0;       // Vertical look (radians)
    
    // Pan mode state
    private double panCameraX = 0.0;
    private double panCameraY = 5.0;
    private double panCameraZ = -10.0;
    private double panTargetX = 0.0;
    private double panTargetY = 0.0;
    
    // Movement speeds
    private double orbitRotateSpeed = 0.005;
    private double flyMoveSpeed = 5.0;
    private double flyLookSpeed = 0.003;
    private double panSpeed = 0.01;
    private double zoomSpeed = 0.1;
    
    // Pre-allocated arrays for zero-allocation getters (reused to avoid per-frame allocations)
    private final double[] cameraPositionCache = new double[3];
    private final double[] cameraTargetCache = new double[3];
    
    // Optional engine reference (for future native camera integration)
    private Object engineRef;
    
    /**
     * Create a new ViewportController.
     */
    public ViewportController() {
        this(null);
    }
    
    /**
     * Create a new ViewportController with engine reference.
     * 
     * @param engine Native engine reference (optional, for future use)
     */
    public ViewportController(Object engine) {
        this.engineRef = engine;
    }
    
    /**
     * Set the camera control mode.
     * 
     * @param mode New camera mode
     */
    public void setMode(Mode mode) {
        if (this.mode != mode) {
            System.out.println("[ViewportController] Switching to " + mode + " mode");
            this.mode = mode;
            
            // Reset drag state when switching modes
            isDragging = false;
            dragButton = null;
        }
    }
    
    /**
     * Get current camera mode.
     */
    public Mode getMode() {
        return mode;
    }
    
    /**
     * Handle mouse pressed event.
     * Call this from viewport's onMousePressed handler.
     */
    public void handleMousePressed(MouseEvent event) {
        isDragging = true;
        dragButton = event.getButton();
        lastMouseX = event.getX();
        lastMouseY = event.getY();
    }
    
    /**
     * Handle mouse dragged event.
     * Call this from viewport's onMouseDragged handler.
     */
    public void handleMouseDragged(MouseEvent event) {
        if (!isDragging) {
            return;
        }
        
        double deltaX = event.getX() - lastMouseX;
        double deltaY = event.getY() - lastMouseY;
        lastMouseX = event.getX();
        lastMouseY = event.getY();
        
        switch (mode) {
            case ORBIT:
                handleOrbitDrag(deltaX, deltaY, dragButton);
                break;
            case FLY:
                handleFlyDrag(deltaX, deltaY, dragButton);
                break;
            case PAN:
                handlePanDrag(deltaX, deltaY, dragButton);
                break;
        }
    }
    
    /**
     * Handle mouse released event.
     * Call this from viewport's onMouseReleased handler.
     */
    public void handleMouseReleased(MouseEvent event) {
        isDragging = false;
        dragButton = null;
    }
    
    /**
     * Handle scroll event (for zooming).
     * Call this from viewport's onScroll handler.
     */
    public void handleScroll(ScrollEvent event) {
        double delta = event.getDeltaY() * zoomSpeed;
        
        switch (mode) {
            case ORBIT:
                // Zoom in/out by changing orbit distance
                orbitDistance = Math.max(0.1, orbitDistance - delta);
                break;
            case FLY:
                // Increase movement speed
                flyMoveSpeed = Math.max(0.1, flyMoveSpeed + delta * 0.1);
                break;
            case PAN:
                // Zoom camera forward/backward
                panCameraZ += delta;
                break;
        }
    }
    
    /**
     * Handle key pressed event.
     * Call this from viewport's onKeyPressed handler.
     */
    public void handleKeyPressed(KeyEvent event) {
        KeyCode code = event.getCode();
        
        if (code == KeyCode.W) keyW = true;
        else if (code == KeyCode.A) keyA = true;
        else if (code == KeyCode.S) keyS = true;
        else if (code == KeyCode.D) keyD = true;
        else if (code == KeyCode.Q) keyQ = true;
        else if (code == KeyCode.E) keyE = true;
        else if (code == KeyCode.SHIFT) keyShift = true;
        
        // Mode switching shortcuts
        else if (code == KeyCode.DIGIT1) setMode(Mode.ORBIT);
        else if (code == KeyCode.DIGIT2) setMode(Mode.FLY);
        else if (code == KeyCode.DIGIT3) setMode(Mode.PAN);
    }
    
    /**
     * Handle key released event.
     * Call this from viewport's onKeyReleased handler.
     */
    public void handleKeyReleased(KeyEvent event) {
        KeyCode code = event.getCode();
        
        if (code == KeyCode.W) keyW = false;
        else if (code == KeyCode.A) keyA = false;
        else if (code == KeyCode.S) keyS = false;
        else if (code == KeyCode.D) keyD = false;
        else if (code == KeyCode.Q) keyQ = false;
        else if (code == KeyCode.E) keyE = false;
        else if (code == KeyCode.SHIFT) keyShift = false;
    }
    
    /**
     * Update camera state based on input.
     * Call this every frame with delta time in seconds.
     * 
     * @param deltaTime Time elapsed since last update (seconds)
     */
    public void update(double deltaTime) {
        switch (mode) {
            case FLY:
                updateFlyCamera(deltaTime);
                break;
            case ORBIT:
                updateOrbitCamera(deltaTime);
                break;
            case PAN:
                updatePanCamera(deltaTime);
                break;
        }
    }
    
    /**
     * Get current camera position (for external use).
     * Returns a reused array [x, y, z] - DO NOT MODIFY.
     * The array contents are updated on each call.
     */
    public double[] getCameraPosition() {
        switch (mode) {
            case ORBIT:
                calculateOrbitPosition(cameraPositionCache);
                break;
            case FLY:
                cameraPositionCache[0] = flyCameraX;
                cameraPositionCache[1] = flyCameraY;
                cameraPositionCache[2] = flyCameraZ;
                break;
            case PAN:
                cameraPositionCache[0] = panCameraX;
                cameraPositionCache[1] = panCameraY;
                cameraPositionCache[2] = panCameraZ;
                break;
            default:
                cameraPositionCache[0] = 0;
                cameraPositionCache[1] = 0;
                cameraPositionCache[2] = 0;
        }
        return cameraPositionCache;
    }
    
    /**
     * Get current camera target/look-at point (for external use).
     * Returns a reused array [x, y, z] - DO NOT MODIFY.
     * The array contents are updated on each call.
     */
    public double[] getCameraTarget() {
        switch (mode) {
            case ORBIT:
                cameraTargetCache[0] = orbitTargetX;
                cameraTargetCache[1] = orbitTargetY;
                cameraTargetCache[2] = orbitTargetZ;
                break;
            case FLY:
                calculateFlyTarget(cameraTargetCache);
                break;
            case PAN:
                cameraTargetCache[0] = panTargetX;
                cameraTargetCache[1] = panTargetY;
                cameraTargetCache[2] = 0;
                break;
            default:
                cameraTargetCache[0] = 0;
                cameraTargetCache[1] = 0;
                cameraTargetCache[2] = 0;
        }
        return cameraTargetCache;
    }
    
    // ========== Internal Implementation ==========
    
    private void handleOrbitDrag(double deltaX, double deltaY, MouseButton button) {
        if (button == MouseButton.PRIMARY) {
            // Left drag: Rotate camera
            orbitAzimuth -= deltaX * orbitRotateSpeed;
            orbitElevation += deltaY * orbitRotateSpeed;
            
            // Clamp elevation to avoid gimbal lock
            orbitElevation = Math.max(-Math.PI / 2 + 0.01, Math.min(Math.PI / 2 - 0.01, orbitElevation));
        } else if (button == MouseButton.MIDDLE) {
            // Middle drag: Pan target
            double panAmount = orbitDistance * 0.001;
            orbitTargetX -= deltaX * panAmount;
            orbitTargetY += deltaY * panAmount;
        }
    }
    
    private void handleFlyDrag(double deltaX, double deltaY, MouseButton button) {
        if (button == MouseButton.PRIMARY) {
            // Left drag: Look around
            flyYaw -= deltaX * flyLookSpeed;
            flyPitch += deltaY * flyLookSpeed;
            
            // Clamp pitch
            flyPitch = Math.max(-Math.PI / 2 + 0.01, Math.min(Math.PI / 2 - 0.01, flyPitch));
        }
    }
    
    private void handlePanDrag(double deltaX, double deltaY, MouseButton button) {
        if (button == MouseButton.PRIMARY || button == MouseButton.MIDDLE) {
            // Drag: Pan camera
            panCameraX -= deltaX * panSpeed;
            panTargetX -= deltaX * panSpeed;
            panCameraY += deltaY * panSpeed;
            panTargetY += deltaY * panSpeed;
        }
    }
    
    private void updateOrbitCamera(double deltaTime) {
        // Orbit mode doesn't need continuous updates (mouse-driven only)
    }
    
    private void updateFlyCamera(double deltaTime) {
        if (!keyW && !keyA && !keyS && !keyD && !keyQ && !keyE) {
            return; // No movement
        }
        
        // Calculate movement speed (with shift modifier)
        double speed = keyShift ? flyMoveSpeed * 2.0 : flyMoveSpeed;
        double moveAmount = speed * deltaTime;
        
        // Calculate forward and right vectors
        double forwardX = Math.sin(flyYaw) * Math.cos(flyPitch);
        double forwardY = -Math.sin(flyPitch);
        double forwardZ = Math.cos(flyYaw) * Math.cos(flyPitch);
        
        double rightX = Math.sin(flyYaw + Math.PI / 2);
        double rightZ = Math.cos(flyYaw + Math.PI / 2);
        
        // Apply movement
        if (keyW) {
            flyCameraX += forwardX * moveAmount;
            flyCameraY += forwardY * moveAmount;
            flyCameraZ += forwardZ * moveAmount;
        }
        if (keyS) {
            flyCameraX -= forwardX * moveAmount;
            flyCameraY -= forwardY * moveAmount;
            flyCameraZ -= forwardZ * moveAmount;
        }
        if (keyA) {
            flyCameraX -= rightX * moveAmount;
            flyCameraZ -= rightZ * moveAmount;
        }
        if (keyD) {
            flyCameraX += rightX * moveAmount;
            flyCameraZ += rightZ * moveAmount;
        }
        if (keyQ) {
            flyCameraY -= moveAmount;
        }
        if (keyE) {
            flyCameraY += moveAmount;
        }
    }
    
    private void updatePanCamera(double deltaTime) {
        // Pan mode doesn't need continuous updates (mouse-driven only)
    }
    
    private void calculateOrbitPosition(double[] out) {
        // Convert spherical coordinates to cartesian
        out[0] = orbitTargetX + orbitDistance * Math.sin(orbitAzimuth) * Math.cos(orbitElevation);
        out[1] = orbitTargetY + orbitDistance * Math.sin(orbitElevation);
        out[2] = orbitTargetZ + orbitDistance * Math.cos(orbitAzimuth) * Math.cos(orbitElevation);
    }
    
    private void calculateFlyTarget(double[] out) {
        // Calculate look-at target point 1 unit ahead
        out[0] = flyCameraX + Math.sin(flyYaw) * Math.cos(flyPitch);
        out[1] = flyCameraY - Math.sin(flyPitch);
        out[2] = flyCameraZ + Math.cos(flyYaw) * Math.cos(flyPitch);
    }
    
    /**
     * Reset camera to default position for current mode.
     */
    public void reset() {
        switch (mode) {
            case ORBIT:
                orbitDistance = 10.0;
                orbitAzimuth = 0.0;
                orbitElevation = 0.5;
                orbitTargetX = orbitTargetY = orbitTargetZ = 0.0;
                break;
            case FLY:
                flyCameraX = 0.0;
                flyCameraY = 5.0;
                flyCameraZ = -10.0;
                flyYaw = 0.0;
                flyPitch = 0.0;
                break;
            case PAN:
                panCameraX = 0.0;
                panCameraY = 5.0;
                panCameraZ = -10.0;
                panTargetX = 0.0;
                panTargetY = 0.0;
                break;
        }
        System.out.println("[ViewportController] Camera reset");
    }
    
    /**
     * Set orbit target position.
     */
    public void setOrbitTarget(double x, double y, double z) {
        orbitTargetX = x;
        orbitTargetY = y;
        orbitTargetZ = z;
    }
    
    /**
     * Set orbit distance from target.
     */
    public void setOrbitDistance(double distance) {
        orbitDistance = Math.max(0.1, distance);
    }
    
    /**
     * Get a debug string describing current camera state.
     */
    public String getDebugInfo() {
        double[] pos = getCameraPosition();
        return String.format("%s mode | Pos: (%.1f, %.1f, %.1f)", 
                           mode, pos[0], pos[1], pos[2]);
    }
    
    /**
     * Sync camera state to a NativeCamera instance.
     * Converts the controller's camera state to a CameraDesc and applies it.
     * 
     * @param nativeCamera The native camera to update
     */
    public void syncToNativeCamera(com.astraeus.native_api.NativeCamera nativeCamera) {
        if (nativeCamera == null) {
            return;
        }
        
        double[] pos = getCameraPosition();
        double[] target = getCameraTarget();
        
        // Get current descriptor
        com.astraeus.native_api.NativeCamera.CameraDesc currentDesc = nativeCamera.getDesc();
        
        // Create updated descriptor with new position and target
        com.astraeus.native_api.NativeCamera.CameraDesc newDesc = 
            new com.astraeus.native_api.NativeCamera.CameraDesc(
                (float) pos[0], (float) pos[1], (float) pos[2],      // position
                (float) target[0], (float) target[1], (float) target[2],  // target
                0.0f, 1.0f, 0.0f,                                     // up vector (always Y-up)
                currentDesc.fovDegrees(),                             // keep FOV
                currentDesc.nearPlane(),                              // keep near
                currentDesc.farPlane(),                               // keep far
                getModeConstant()                                     // camera mode
            );
        
        // Apply to native camera
        nativeCamera.setDesc(newDesc);
    }
    
    /**
     * Get the native API constant for the current mode.
     */
    private int getModeConstant() {
        switch (mode) {
            case ORBIT:
                return com.astraeus.native_api.EngineBindings.CAMERA_MODE_ORBIT;
            case FLY:
                return com.astraeus.native_api.EngineBindings.CAMERA_MODE_FLY;
            case PAN:
                return com.astraeus.native_api.EngineBindings.CAMERA_MODE_PAN;
            default:
                return com.astraeus.native_api.EngineBindings.CAMERA_MODE_ORBIT;
        }
    }
}
