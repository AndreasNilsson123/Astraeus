# Resize & Camera Propagation Flow

**Document**: VIS-GEN-001 Resize & Camera Validation  
**Purpose**: Define deterministic resize and camera update contracts  
**Scope**: Viewport resize → Camera aspect → Render → Readback → Present

---

## 1. Overview

Viewport resizing is a common source of visual artifacts if not handled correctly. This document defines the **deterministic resize flow** that ensures:

- Camera aspect ratio matches viewport dimensions
- No torn frames with partial updates
- No stale viewport/scissor state
- Consistent readback dimensions

### 1.1 Key Invariants

```
INV-1: camera.aspect_ratio == viewport.width / viewport.height
INV-2: frame(n).viewport == frame(n).camera == frame(n).readback
INV-3: resize complete before next frame begins
INV-4: glViewport and glScissor match current dimensions
```

---

## 2. Resize Flow Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    User Action                          │
│         (Window resize, pane resize, maximize)          │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│  JavaFX: ViewportPane.widthProperty/heightProperty      │
│  Listener triggered with new dimensions                 │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│  Java: FxViewport.resizeViewport(newW, newH)            │
│  1. Update internal dimensions                          │
│  2. Recreate PixelBuffer if needed (backing size ok)    │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│  FFM: NativeViewport.resizeWithProjection(w, h, ...)    │
│  Single atomic operation: resize + update camera        │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│  Native: astraeus_viewport_resize(viewport, w, h)       │
│  1. Update RenderDevice viewport dimensions             │
│  2. Update framebuffer viewport (color, ID buffers)     │
│  3. Clamp to max backing buffer dimensions              │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│  Native: Update Camera Projection                       │
│  1. Get active camera from viewport                     │
│  2. Calculate new aspect = w / h                        │
│  3. Update camera projection matrix                     │
│  4. Mark render state as dirty                          │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│  Next Frame: astraeus_begin_frame()                     │
│  1. Apply new viewport/scissor to GL state              │
│  2. Render with updated camera projection               │
│  3. Readback reflects new dimensions                    │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│  Java: updateDisplay()                                  │
│  1. Get updated PixelBufferView                         │
│  2. Update dirty region to new dimensions               │
│  3. JavaFX renders with correct size                    │
└─────────────────────────────────────────────────────────┘
```

---

## 3. Implementation Details

### 3.1 JavaFX Resize Handler

```java
public class FxViewport extends StackPane {
    
    private final NativeViewport nativeViewport;
    private int currentWidth;
    private int currentHeight;
    
    /**
     * Resize viewport to new dimensions.
     * This is the entry point for all resize operations.
     * 
     * CRITICAL: Must update camera projection in same call to prevent
     * torn frames where viewport != camera aspect.
     */
    public void resizeViewport(int newWidth, int newHeight) {
        if (newWidth == currentWidth && newHeight == currentHeight) {
            return; // No change
        }
        
        // Clamp to backing buffer max dimensions
        int clampedWidth = Math.min(newWidth, maxWidth);
        int clampedHeight = Math.min(newHeight, maxHeight);
        
        if (clampedWidth != newWidth || clampedHeight != newHeight) {
            System.err.println("Warning: Resize clamped from " + 
                newWidth + "x" + newHeight + " to " +
                clampedWidth + "x" + clampedHeight);
        }
        
        // ATOMIC: Resize viewport AND update camera projection
        // This prevents frame(n) from having viewport != camera aspect
        nativeViewport.resizeWithProjection(
            clampedWidth, clampedHeight,
            controller.getFOV(),
            controller.getNearPlane(),
            controller.getFarPlane()
        );
        
        // Update Java-side dimensions
        currentWidth = clampedWidth;
        currentHeight = clampedHeight;
        
        // Recreate PixelBuffer dirty region
        updateDirtyRegion();
        
        // Log for diagnostics
        if (diagnosticMode) {
            System.out.println("Viewport resized: " + currentWidth + "x" + currentHeight);
        }
    }
    
    /**
     * Update dirty region for PixelBuffer.
     * JavaFX needs to know which region changed.
     */
    private void updateDirtyRegion() {
        // Full viewport is dirty on resize
        dirtyRect = new Rectangle2D(0, 0, currentWidth, currentHeight);
        lastDirtyW = currentWidth;
        lastDirtyH = currentHeight;
    }
}
```

### 3.2 Native Viewport Resize

```cpp
// In NativeViewport or EngineContext

AstraeusResult astraeus_viewport_resize(ViewportHandle viewport, 
                                         uint32_t width, uint32_t height) {
    if (!viewport) {
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    NativeViewport* vp = reinterpret_cast<NativeViewport*>(viewport);
    
    // Clamp to max backing buffer dimensions
    uint32_t max_w = vp->color_buffer_->get_max_backing_width();
    uint32_t max_h = vp->color_buffer_->get_max_backing_height();
    
    uint32_t clamped_w = std::min(width, max_w);
    uint32_t clamped_h = std::min(height, max_h);
    
    if (clamped_w != width || clamped_h != height) {
        std::cerr << "[Viewport] Warning: Resize clamped from " 
                  << width << "x" << height << " to "
                  << clamped_w << "x" << clamped_h << std::endl;
    }
    
    // Update RenderDevice viewport dimensions
    // NOTE: This does NOT reallocate buffers, only updates viewport region
    vp->render_device_->set_viewport_dimensions(clamped_w, clamped_h);
    
    // Update framebuffer viewport regions (color and ID)
    vp->color_buffer_->set_viewport_size(clamped_w, clamped_h);
    vp->id_buffer_->set_viewport_size(clamped_w, clamped_h);
    
    // Mark render state as dirty (will be applied at next begin_frame)
    vp->render_device_->mark_state_dirty();
    
    return ASTRAEUS_SUCCESS;
}
```

### 3.3 Camera Projection Update

```cpp
// In NativeViewport (called from resizeWithProjection Java wrapper)

AstraeusResult update_camera_projection_internal(
    ViewportHandle viewport,
    float fov_degrees,
    float near_plane,
    float far_plane) 
{
    if (!viewport) {
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    NativeViewport* vp = reinterpret_cast<NativeViewport*>(viewport);
    
    // Get current viewport dimensions
    uint32_t vp_width = vp->render_device_->get_viewport_width();
    uint32_t vp_height = vp->render_device_->get_viewport_height();
    
    // Calculate aspect ratio from viewport
    float aspect_ratio = static_cast<float>(vp_width) / 
                         static_cast<float>(vp_height);
    
    // Validate parameters
    if (fov_degrees <= 0.0f || fov_degrees >= 180.0f) {
        std::cerr << "[Camera] Invalid FOV: " << fov_degrees << std::endl;
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    if (near_plane <= 0.0f || far_plane <= near_plane) {
        std::cerr << "[Camera] Invalid clipping planes: near=" 
                  << near_plane << ", far=" << far_plane << std::endl;
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    // Get active camera
    Camera* camera = vp->camera_;
    if (!camera) {
        return ASTRAEUS_ERROR_NOT_INITIALIZED;
    }
    
    // Update camera projection
    camera->set_perspective(fov_degrees, aspect_ratio, near_plane, far_plane);
    
    // CRITICAL: Camera projection matrix will be applied at next begin_frame()
    // This ensures all render passes use the updated projection
    
    return ASTRAEUS_SUCCESS;
}
```

### 3.4 Render State Application

```cpp
// In RenderDevice::begin_frame()

void RenderDevice::begin_frame() {
    // Apply viewport and scissor if state is dirty
    if (state_dirty_) {
        apply_viewport_state();
        state_dirty_ = false;
    }
    
    // ... rest of begin_frame logic
}

void RenderDevice::apply_viewport_state() {
    // CRITICAL: Update both viewport and scissor
    // Stale scissor state causes partial-frame rendering
    
    glViewport(0, 0, viewport_width_, viewport_height_);
    glScissor(0, 0, viewport_width_, viewport_height_);
    
    if (diagnostic_mode_) {
        std::cout << "[RenderDevice] Viewport/Scissor: " 
                  << viewport_width_ << "x" << viewport_height_ << std::endl;
    }
}
```

---

## 4. Atomic Resize + Camera Update

### 4.1 Java Wrapper API

```java
public class NativeViewport implements AutoCloseable {
    
    /**
     * Resize viewport and update camera projection atomically.
     * 
     * This is the CORRECT way to resize. It ensures the camera
     * projection is updated with the correct aspect ratio derived
     * from the new viewport dimensions.
     * 
     * DO NOT call resize() separately and then try to update camera.
     * This can cause torn frames where viewport != camera aspect.
     * 
     * @param width New viewport width (device pixels)
     * @param height New viewport height (device pixels)
     * @param fovDegrees Field of view in degrees
     * @param nearPlane Near clipping plane
     * @param farPlane Far clipping plane
     */
    public void resizeWithProjection(int width, int height,
                                      float fovDegrees, 
                                      float nearPlane, 
                                      float farPlane) {
        checkClosed();
        
        // Step 1: Resize viewport (updates dimensions, marks state dirty)
        resize(width, height);
        
        // Step 2: Update camera projection (uses new aspect from Step 1)
        updateCameraProjection(fovDegrees, nearPlane, farPlane);
        
        // Both operations complete before next frame begins
        // Invariant: camera.aspect == viewport.width / viewport.height
    }
    
    /**
     * Update camera projection using current viewport dimensions.
     * Called internally by resizeWithProjection.
     */
    private void updateCameraProjection(float fovDegrees, 
                                        float nearPlane, 
                                        float farPlane) {
        try {
            int result = (int) EngineBindings.UPDATE_CAMERA_PROJECTION.invoke(
                viewportHandle, fovDegrees, nearPlane, farPlane);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException(
                    "Failed to update camera projection, error: " + result);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to update camera", e);
        }
    }
}
```

### 4.2 Native Implementation

```cpp
// Add to EngineAPI.h

/**
 * Update camera projection matrix.
 * Uses current viewport dimensions to calculate aspect ratio.
 * Should be called after viewport resize to ensure camera matches viewport.
 * 
 * @param viewport Viewport handle
 * @param fov_degrees Field of view in degrees
 * @param near_plane Near clipping plane distance
 * @param far_plane Far clipping plane distance
 * @return ASTRAEUS_SUCCESS or error code
 */
ASTRAEUS_API AstraeusResult astraeus_viewport_update_camera_projection(
    ViewportHandle viewport,
    float fov_degrees,
    float near_plane,
    float far_plane);
```

**Implementation**:
```cpp
AstraeusResult astraeus_viewport_update_camera_projection(
    ViewportHandle viewport,
    float fov_degrees,
    float near_plane,
    float far_plane) 
{
    return update_camera_projection_internal(viewport, 
        fov_degrees, near_plane, far_plane);
}
```

---

## 5. Testing Resize Correctness

### 5.1 Automated Resize Test

```java
@Test
public void testResizeCameraAspectSync() {
    NativeEngine engine = new NativeEngine(800, 600, true);
    NativeViewport viewport = engine.createViewport(800, 600);
    
    // Initial state: camera aspect should match viewport
    float initialAspect = 800.0f / 600.0f;
    CameraDesc camera = viewport.getActiveCamera().getDesc();
    assertEquals(initialAspect, camera.getAspectRatio(), 0.01f);
    
    // Resize to 16:9
    viewport.resizeWithProjection(1920, 1080, 60.0f, 0.1f, 1000.0f);
    
    // Render a frame to apply state
    engine.beginFrame(0.016);
    engine.endFrame();
    
    // Verify camera aspect updated
    camera = viewport.getActiveCamera().getDesc();
    float expectedAspect = 1920.0f / 1080.0f;
    assertEquals(expectedAspect, camera.getAspectRatio(), 0.01f);
    
    // Resize to 4:3
    viewport.resizeWithProjection(1024, 768, 60.0f, 0.1f, 1000.0f);
    
    engine.beginFrame(0.016);
    engine.endFrame();
    
    camera = viewport.getActiveCamera().getDesc();
    expectedAspect = 1024.0f / 768.0f;
    assertEquals(expectedAspect, camera.getAspectRatio(), 0.01f);
    
    viewport.close();
    engine.close();
}
```

### 5.2 Manual Validation

**Checklist**:

- [ ] Resize window multiple times rapidly
- [ ] Check no visual distortion (stretched/squashed)
- [ ] Verify camera controls work correctly after resize
- [ ] Check debug overlay shows matching viewport/camera aspect
- [ ] Test edge cases: minimize, maximize, fullscreen

**Visual Test Pattern**:
```java
// Use test pattern to verify no distortion
engine.setTestPattern(TestPatternType.QUADRANTS);
// Resize window
// Verify quadrants remain square (not rectangular)
// Red and green should be on top, blue and yellow on bottom
```

---

## 6. Common Resize Issues

### 6.1 Issue: Stale Viewport State

**Symptom**: Partial rendering, black regions, or clipped scene

**Cause**: `glViewport()` or `glScissor()` not updated after resize

**Fix**:
```cpp
void RenderDevice::apply_viewport_state() {
    // ALWAYS update both
    glViewport(0, 0, viewport_width_, viewport_height_);
    glScissor(0, 0, viewport_width_, viewport_height_);
}
```

### 6.2 Issue: Camera Aspect Mismatch

**Symptom**: Scene appears stretched or squashed after resize

**Cause**: Camera projection not updated with new aspect ratio

**Fix**: Use `resizeWithProjection()` instead of separate `resize()` + `updateCamera()`

### 6.3 Issue: Torn Frames

**Symptom**: One frame shows old size, next shows new size (visual "pop")

**Cause**: Resize applied mid-frame or state not synchronized

**Fix**: Ensure resize completes before `begin_frame()`, apply state atomically

### 6.4 Issue: Buffer Overflow

**Symptom**: Crash or visual corruption when resizing beyond max dimensions

**Cause**: Viewport dimensions exceed backing buffer max size

**Fix**: Clamp resize requests to max backing dimensions

```cpp
uint32_t clamped_w = std::min(requested_width, max_backing_width);
uint32_t clamped_h = std::min(requested_height, max_backing_height);
```

---

## 7. Diagnostic Validation

### 7.1 Per-Frame Resize Validation

```cpp
// In EngineContext::validate_frame_state()

void validate_resize_state(FrameDiagnostics& diag) {
    uint32_t vp_w = render_device_->get_viewport_width();
    uint32_t vp_h = render_device_->get_viewport_height();
    
    float vp_aspect = static_cast<float>(vp_w) / vp_h;
    float cam_aspect = camera_->get_aspect_ratio();
    
    // Check aspect ratio match
    if (std::abs(vp_aspect - cam_aspect) > 0.01f) {
        diag.validation_flags |= DIAG_FLAG_CAMERA_ASPECT_WRONG;
        snprintf(diag.warning_message, sizeof(diag.warning_message),
                 "Resize validation failed: viewport aspect %.3f != camera aspect %.3f",
                 vp_aspect, cam_aspect);
    }
    
    // Check viewport within backing buffer
    if (vp_w > diag.backing_width || vp_h > diag.backing_height) {
        diag.validation_flags |= DIAG_FLAG_SIZE_MISMATCH;
        snprintf(diag.error_message, sizeof(diag.error_message),
                 "Viewport (%u x %u) exceeds backing buffer (%u x %u)",
                 vp_w, vp_h, diag.backing_width, diag.backing_height);
    }
}
```

### 7.2 Java Diagnostic Overlay

```java
// In DiagnosticOverlayPane

private void updateResizeValidation(FrameDiagnosticsJava diag) {
    float vpAspect = (float) diag.getViewportWidth() / diag.getViewportHeight();
    float camAspect = diag.getCameraAspect();
    
    boolean aspectMatch = Math.abs(vpAspect - camAspect) < 0.01f;
    
    String status = aspectMatch ? "✓" : "✗";
    String color = aspectMatch ? "green" : "red";
    
    resizeStatusLabel.setText(String.format(
        "%s Resize: VP=%.3f, Cam=%.3f", 
        status, vpAspect, camAspect));
    resizeStatusLabel.setTextFill(Color.web(color));
}
```

---

## 8. Best Practices

### 8.1 DO

✅ Use `resizeWithProjection()` for atomic resize + camera update  
✅ Clamp resize requests to max backing buffer dimensions  
✅ Apply viewport/scissor state at `begin_frame()`  
✅ Validate camera aspect matches viewport in diagnostic mode  
✅ Test resize at various sizes and aspect ratios  

### 8.2 DON'T

❌ Call `resize()` without updating camera projection  
❌ Update camera mid-frame (only at frame boundaries)  
❌ Exceed max backing buffer dimensions  
❌ Forget to update scissor rect (not just viewport)  
❌ Assume resize completes immediately (apply at next frame)  

---

## 9. Frame Timing Diagram

```
Frame N-1:  [resize request queued]
                │
                ▼
Frame N:    begin_frame()
                ├─ apply_viewport_state() <-- New dimensions applied here
                ├─ update_camera_projection() <-- New aspect applied here
                ├─ render_passes() <-- Uses new viewport + camera
                ├─ readback() <-- Reads new dimensions
                └─ end_frame()
                │
                ▼
            updateDisplay() <-- JavaFX shows new size
                │
                ▼
Frame N+1:  [resize fully visible, no artifacts]
```

**Key Points**:
- Resize request may happen mid-frame N-1
- State changes applied atomically at begin_frame(N)
- All of frame N uses consistent dimensions
- JavaFX sees correct size after end_frame(N)

---

## 10. Summary

**Critical Requirements**:

1. **Atomic Update**: Viewport dimensions and camera aspect MUST be updated together
2. **State Synchronization**: All render state (viewport, scissor, projection) applied at frame boundaries
3. **Clamping**: Resize requests MUST be clamped to max backing buffer dimensions
4. **Validation**: Per-frame checks ensure viewport and camera remain synchronized

**Verification**:

✅ Automated tests verify camera aspect after resize  
✅ Manual testing with various sizes and aspect ratios  
✅ Diagnostic overlay shows real-time validation status  
✅ Test patterns verify no visual distortion  
✅ Stress test with rapid resize operations  

**API Summary**:

- **Java**: `viewport.resizeWithProjection(w, h, fov, near, far)` - Use this!
- **Native**: `astraeus_viewport_resize()` + `astraeus_viewport_update_camera_projection()`
- **Validation**: `FrameDiagnostics.validation_flags` - Check `DIAG_FLAG_CAMERA_ASPECT_WRONG`
