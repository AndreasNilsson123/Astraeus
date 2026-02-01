# Visualization Diagnostics Framework

**Document**: VIS-GEN-001 Diagnostics & Instrumentation  
**Purpose**: Define unified per-frame diagnostic snapshot and observability tools  
**Scope**: Real-time visualization correctness validation

---

## 1. Overview

The Diagnostics Framework provides comprehensive, real-time visibility into the visualization pipeline state. It enables:

- **Early detection** of correctness violations
- **Root cause analysis** of visual artifacts
- **Performance monitoring** without production overhead
- **Automated regression testing** with verification data

### 1.1 Design Principles

1. **Zero-overhead when disabled** - no cost in release builds
2. **Minimal overhead when enabled** - <1% frame time impact
3. **Unified snapshot** - all diagnostic data in single coherent view
4. **Observable invariants** - make hard contracts verifiable
5. **Actionable errors** - clear guidance for debugging

---

## 2. Core Diagnostic Struct

### 2.1 FrameDiagnostics (Native)

```c
typedef struct {
    // Frame identity
    uint64_t frame_number;
    double timestamp_ms;
    
    // Viewport dimensions (device pixels)
    uint32_t viewport_width;
    uint32_t viewport_height;
    
    // Backing buffer dimensions (fixed at init)
    uint32_t backing_width;
    uint32_t backing_height;
    uint32_t backing_size_bytes;
    
    // Color buffer info
    void* color_buffer_ptr;
    uint32_t color_format;      // PixelFormat enum
    uint32_t color_stride;
    uint32_t color_bytes_per_pixel;
    uint32_t color_readback_bytes;
    
    // ID buffer info
    void* id_buffer_ptr;
    uint32_t id_format;
    uint32_t id_stride;
    uint32_t id_bytes_per_pixel;
    uint32_t id_readback_bytes;
    
    // Camera state
    float camera_fov_degrees;
    float camera_aspect_ratio;
    float camera_near_plane;
    float camera_far_plane;
    vec3 camera_position;
    vec3 camera_target;
    
    // Scene state
    uint32_t total_entity_count;
    uint32_t visible_entity_count;
    uint32_t submitted_entity_count;  // Actually submitted to render passes
    
    // Render stats
    uint32_t draw_calls;
    uint32_t triangle_count;
    uint32_t active_pass_count;
    
    // Timing (in milliseconds)
    double cpu_time_ms;
    double gpu_time_ms;  // May be 0 if not available
    double total_frame_ms;
    
    // Pass timings (optional, limited to 16 passes)
    uint32_t pass_timing_count;
    PassTiming pass_timings[16];
    
    // Validation flags (bitmask)
    uint32_t validation_flags;  // See ValidationFlags enum
    
    // Error/warning messages (first error in frame)
    char error_message[256];
    char warning_message[256];
    
} FrameDiagnostics;
```

### 2.2 ValidationFlags Enum

```c
typedef enum {
    DIAG_FLAG_NONE                  = 0,
    DIAG_FLAG_SIZE_MISMATCH         = 1 << 0,   // Viewport != backing or inconsistent
    DIAG_FLAG_CAMERA_ASPECT_WRONG   = 1 << 1,   // Camera aspect != viewport aspect
    DIAG_FLAG_BUFFER_OVERFLOW       = 1 << 2,   // Readback exceeds backing buffer
    DIAG_FLAG_STRIDE_INVALID        = 1 << 3,   // Stride < width * bpp
    DIAG_FLAG_FORMAT_MISMATCH       = 1 << 4,   // Unexpected pixel format
    DIAG_FLAG_STALE_VIEWPORT        = 1 << 5,   // Viewport/scissor not updated
    DIAG_FLAG_ENTITY_COUNT_MISMATCH = 1 << 6,   // Entity counts inconsistent
    DIAG_FLAG_INVALID_TRANSFORM     = 1 << 7,   // NaN/Inf in transform
    DIAG_FLAG_TEST_PATTERN_ACTIVE   = 1 << 8,   // Test pattern mode enabled
    DIAG_FLAG_DOUBLE_BUFFER_SWAP    = 1 << 9,   // Double buffer swap occurred
    DIAG_FLAG_RESIZE_OCCURRED       = 1 << 10,  // Viewport resized this frame
} ValidationFlags;
```

### 2.3 PassTiming Struct

```c
typedef struct {
    char pass_name[32];
    double duration_ms;
    uint32_t draw_calls;
    uint32_t entities_submitted;
} PassTiming;
```

---

## 3. API Functions

### 3.1 Diagnostic Control

```c
/**
 * Enable or disable diagnostic mode.
 * When enabled, detailed per-frame snapshots are captured.
 * @param engine Engine handle
 * @param enable true to enable, false to disable
 */
ASTRAEUS_API void astraeus_set_diagnostic_mode(EngineHandle engine, bool enable);

/**
 * Get current diagnostic mode state.
 */
ASTRAEUS_API bool astraeus_get_diagnostic_mode(EngineHandle engine);

/**
 * Set validation level (0=none, 1=basic, 2=full, 3=paranoid).
 * Higher levels perform more expensive checks.
 */
ASTRAEUS_API void astraeus_set_validation_level(EngineHandle engine, uint32_t level);
```

### 3.2 Diagnostic Data Retrieval

```c
/**
 * Get comprehensive frame diagnostics.
 * Call once per frame after end_frame().
 * @param engine Engine handle
 * @param out_diag Output diagnostics (must not be NULL)
 */
ASTRAEUS_API void astraeus_get_frame_diagnostics(EngineHandle engine, 
                                                  FrameDiagnostics* out_diag);

/**
 * Get last validation error message.
 * @return Error string or NULL if no error
 */
ASTRAEUS_API const char* astraeus_get_validation_error(EngineHandle engine);
```

### 3.3 Test Pattern Control

```c
/**
 * Test pattern types for validation.
 */
typedef enum {
    TEST_PATTERN_NONE = 0,
    TEST_PATTERN_QUADRANTS,     // TL=Red, TR=Green, BL=Blue, BR=Yellow
    TEST_PATTERN_CHECKERBOARD,  // Black/white squares
    TEST_PATTERN_GRADIENT,      // Horizontal red-to-green gradient
    TEST_PATTERN_GRID,          // Red vertical, green horizontal lines
    TEST_PATTERN_COLOR_BANDS,   // Horizontal color bands
} TestPatternType;

/**
 * Enable test pattern rendering instead of normal scene.
 * Used to isolate presentation issues from scene rendering issues.
 * @param engine Engine handle
 * @param pattern Pattern type (TEST_PATTERN_NONE to disable)
 * @return true on success
 */
ASTRAEUS_API bool astraeus_set_test_pattern(EngineHandle engine, TestPatternType pattern);

/**
 * Get current test pattern type.
 */
ASTRAEUS_API TestPatternType astraeus_get_test_pattern(EngineHandle engine);
```

---

## 4. Java Diagnostic API

### 4.1 NativeEngine Additions

```java
public class NativeEngine {
    
    /**
     * Enable diagnostic mode with validation.
     */
    public void setDiagnosticMode(boolean enable) {
        // Call native astraeus_set_diagnostic_mode
    }
    
    /**
     * Set validation level (0-3).
     */
    public void setValidationLevel(int level) {
        // Call native astraeus_set_validation_level
    }
    
    /**
     * Get comprehensive frame diagnostics.
     */
    public FrameDiagnosticsJava getFrameDiagnostics() {
        // Call native astraeus_get_frame_diagnostics
        // Return Java wrapper object
    }
    
    /**
     * Enable test pattern rendering.
     */
    public void setTestPattern(TestPatternType pattern) {
        // Call native astraeus_set_test_pattern
    }
}
```

### 4.2 FrameDiagnosticsJava Wrapper

```java
public class FrameDiagnosticsJava {
    private final long frameNumber;
    private final double timestampMs;
    
    // Viewport info
    private final int viewportWidth;
    private final int viewportHeight;
    private final int backingWidth;
    private final int backingHeight;
    
    // Buffer info
    private final long colorBufferPtr;
    private final int colorFormat;
    private final int colorStride;
    private final int colorReadbackBytes;
    
    // Camera info
    private final float cameraFov;
    private final float cameraAspect;
    
    // Scene counts
    private final int totalEntities;
    private final int visibleEntities;
    private final int submittedEntities;
    
    // Stats
    private final int drawCalls;
    private final int triangles;
    
    // Timing
    private final double cpuTimeMs;
    private final double gpuTimeMs;
    
    // Validation
    private final int validationFlags;
    private final String errorMessage;
    private final String warningMessage;
    
    // Getters...
    
    /**
     * Check if specific validation flag is set.
     */
    public boolean hasValidationFlag(ValidationFlag flag) {
        return (validationFlags & flag.value) != 0;
    }
    
    /**
     * Get human-readable validation summary.
     */
    public String getValidationSummary() {
        if (validationFlags == 0) return "OK";
        
        List<String> issues = new ArrayList<>();
        if (hasValidationFlag(ValidationFlag.SIZE_MISMATCH)) 
            issues.add("Size mismatch");
        if (hasValidationFlag(ValidationFlag.CAMERA_ASPECT_WRONG)) 
            issues.add("Camera aspect incorrect");
        // ... etc
        
        return String.join(", ", issues);
    }
}
```

---

## 5. Diagnostic Overlay (JavaFX)

### 5.1 DiagnosticOverlayPane

```java
/**
 * Comprehensive diagnostic overlay showing real-time pipeline state.
 * Displays unified frame diagnostics with color-coded warnings.
 */
public class DiagnosticOverlayPane extends VBox {
    
    // Layout: 3 columns
    // Left: Viewport/Buffer info
    // Center: Scene/Render info  
    // Right: Validation status
    
    private Label frameNumberLabel;
    private Label viewportSizeLabel;
    private Label backingSizeLabel;
    private Label camerAspectLabel;
    
    private Label entityCountsLabel;
    private Label drawCallsLabel;
    private Label timingLabel;
    
    private Label validationStatusLabel;
    private TextArea errorMessageArea;
    
    /**
     * Update overlay with latest diagnostics.
     * Color-codes values that violate invariants.
     */
    public void update(FrameDiagnosticsJava diag) {
        frameNumberLabel.setText("Frame: " + diag.getFrameNumber());
        
        // Viewport size (highlight if mismatched)
        String vpSize = String.format("%d x %d", 
            diag.getViewportWidth(), diag.getViewportHeight());
        viewportSizeLabel.setText("Viewport: " + vpSize);
        
        boolean sizeMismatch = diag.hasValidationFlag(
            ValidationFlag.SIZE_MISMATCH);
        viewportSizeLabel.setTextFill(sizeMismatch ? Color.RED : Color.WHITE);
        
        // Camera aspect (highlight if wrong)
        float expectedAspect = (float)diag.getViewportWidth() / 
                               diag.getViewportHeight();
        float actualAspect = diag.getCameraAspect();
        boolean aspectWrong = Math.abs(expectedAspect - actualAspect) > 0.01f;
        
        String aspectText = String.format("Camera Aspect: %.3f (expected: %.3f)",
            actualAspect, expectedAspect);
        cameraAspectLabel.setText(aspectText);
        cameraAspectLabel.setTextFill(aspectWrong ? Color.ORANGE : Color.WHITE);
        
        // Entity counts
        String entityText = String.format(
            "Entities: %d total, %d visible, %d submitted",
            diag.getTotalEntities(), 
            diag.getVisibleEntities(),
            diag.getSubmittedEntities());
        entityCountsLabel.setText(entityText);
        
        // Validation status
        if (diag.getValidationFlags() == 0) {
            validationStatusLabel.setText("✓ All checks passed");
            validationStatusLabel.setTextFill(Color.GREEN);
        } else {
            validationStatusLabel.setText("⚠ " + diag.getValidationSummary());
            validationStatusLabel.setTextFill(Color.YELLOW);
        }
        
        // Error messages
        if (!diag.getErrorMessage().isEmpty()) {
            errorMessageArea.setText("ERROR: " + diag.getErrorMessage());
            errorMessageArea.setStyle("-fx-text-fill: red;");
        } else if (!diag.getWarningMessage().isEmpty()) {
            errorMessageArea.setText("WARNING: " + diag.getWarningMessage());
            errorMessageArea.setStyle("-fx-text-fill: orange;");
        } else {
            errorMessageArea.clear();
        }
    }
}
```

### 5.2 Integration with Viewport

```java
// In FxViewport or ViewportPaneV2
private DiagnosticOverlayPane diagnosticOverlay;

public void enableDiagnostics(boolean enable) {
    if (enable && diagnosticOverlay == null) {
        diagnosticOverlay = new DiagnosticOverlayPane();
        diagnosticOverlay.setMouseTransparent(true);
        overlayStack.addOverlay(diagnosticOverlay, OverlayLayer.DEBUG);
    }
    
    if (diagnosticOverlay != null) {
        diagnosticOverlay.setVisible(enable);
    }
    
    engine.setDiagnosticMode(enable);
}

// In render loop
if (diagnosticMode) {
    FrameDiagnosticsJava diag = engine.getFrameDiagnostics();
    diagnosticOverlay.update(diag);
}
```

---

## 6. Validation Implementation

### 6.1 Native Validation (C++)

```cpp
// In EngineContext or RenderDevice

void EngineContext::validate_frame_state(FrameDiagnostics& diag) {
    diag.validation_flags = DIAG_FLAG_NONE;
    diag.error_message[0] = '\0';
    diag.warning_message[0] = '\0';
    
    uint32_t vp_width = render_device_->get_viewport_width();
    uint32_t vp_height = render_device_->get_viewport_height();
    
    // Check viewport vs backing buffer
    if (vp_width > diag.backing_width || vp_height > diag.backing_height) {
        diag.validation_flags |= DIAG_FLAG_SIZE_MISMATCH;
        snprintf(diag.error_message, sizeof(diag.error_message),
                 "Viewport (%u x %u) exceeds backing buffer (%u x %u)",
                 vp_width, vp_height, diag.backing_width, diag.backing_height);
    }
    
    // Check camera aspect ratio
    float vp_aspect = (float)vp_width / vp_height;
    float cam_aspect = camera_->get_aspect_ratio();
    if (std::abs(vp_aspect - cam_aspect) > 0.01f) {
        diag.validation_flags |= DIAG_FLAG_CAMERA_ASPECT_WRONG;
        snprintf(diag.warning_message, sizeof(diag.warning_message),
                 "Camera aspect (%.3f) != viewport aspect (%.3f)",
                 cam_aspect, vp_aspect);
    }
    
    // Check stride validity
    uint32_t min_stride = vp_width * diag.color_bytes_per_pixel;
    if (diag.color_stride < min_stride) {
        diag.validation_flags |= DIAG_FLAG_STRIDE_INVALID;
        snprintf(diag.error_message, sizeof(diag.error_message),
                 "Color stride (%u) < minimum (%u)",
                 diag.color_stride, min_stride);
    }
    
    // Check entity count consistency
    uint32_t total = world_->get_entity_count();
    uint32_t visible = world_->get_visible_entity_count();
    if (visible > total) {
        diag.validation_flags |= DIAG_FLAG_ENTITY_COUNT_MISMATCH;
        snprintf(diag.error_message, sizeof(diag.error_message),
                 "Visible entities (%u) > total entities (%u)",
                 visible, total);
    }
    
    // Paranoid checks (validation_level >= 3)
    if (validation_level_ >= 3) {
        // Check for NaN/Inf in transforms
        for (auto& [id, transform] : world_->get_transforms()) {
            if (!std::isfinite(transform.position.x) ||
                !std::isfinite(transform.position.y) ||
                !std::isfinite(transform.position.z)) {
                diag.validation_flags |= DIAG_FLAG_INVALID_TRANSFORM;
                snprintf(diag.warning_message, sizeof(diag.warning_message),
                         "Entity %u has invalid transform (NaN/Inf)", id);
                break;
            }
        }
    }
}
```

### 6.2 Validation Levels

**Level 0: None**
- No validation checks
- Zero overhead

**Level 1: Basic (Default in debug builds)**
- Size/stride checks
- Camera aspect validation
- Entity count consistency

**Level 2: Full**
- All Level 1 checks
- Render state validation
- Buffer pointer stability checks

**Level 3: Paranoid**
- All Level 2 checks
- Per-entity transform validation (NaN/Inf)
- Detailed memory layout verification
- Performance impact: ~2-5% frame time

---

## 7. Test Pattern Implementation

### 7.1 Native Test Pattern Rendering

```cpp
// In RenderDevice or dedicated TestPatternPass

void RenderDevice::render_test_pattern(TestPatternType pattern) {
    // Render directly to color buffer
    switch (pattern) {
        case TEST_PATTERN_QUADRANTS:
            render_quadrants();
            break;
        case TEST_PATTERN_CHECKERBOARD:
            render_checkerboard(32); // 32x32 pixel squares
            break;
        case TEST_PATTERN_GRADIENT:
            render_gradient();
            break;
        case TEST_PATTERN_GRID:
            render_grid(50); // 50 pixel spacing
            break;
        case TEST_PATTERN_COLOR_BANDS:
            render_color_bands(60); // 60 pixel height per band
            break;
    }
}

void RenderDevice::render_quadrants() {
    // Clear to black first
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    uint32_t w = viewport_width_;
    uint32_t h = viewport_height_;
    uint32_t mid_x = w / 2;
    uint32_t mid_y = h / 2;
    
    // Use simple shader to render colored quads
    // Top-left: Red (1, 0, 0)
    render_quad(0, mid_y, mid_x, h, vec3(1, 0, 0));
    
    // Top-right: Green (0, 1, 0)
    render_quad(mid_x, mid_y, w, h, vec3(0, 1, 0));
    
    // Bottom-left: Blue (0, 0, 1)
    render_quad(0, 0, mid_x, mid_y, vec3(0, 0, 1));
    
    // Bottom-right: Yellow (1, 1, 0)
    render_quad(mid_x, 0, w, mid_y, vec3(1, 1, 0));
}
```

### 7.2 Java Test Pattern Verification

```java
/**
 * Verify test pattern is rendered correctly.
 * Used for automated testing.
 */
public class TestPatternVerifier {
    
    public static boolean verifyQuadrants(ByteBuffer buffer, 
                                          int width, int height, int stride) {
        int midX = width / 2;
        int midY = height / 2;
        
        // Check top-left corner (should be red)
        Color tlColor = getPixelColor(buffer, 10, height - 10, width, stride);
        if (!isApproximately(tlColor, Color.RED, 10)) {
            System.err.println("Top-left quadrant failed: expected red, got " + tlColor);
            return false;
        }
        
        // Check top-right corner (should be green)
        Color trColor = getPixelColor(buffer, width - 10, height - 10, width, stride);
        if (!isApproximately(trColor, Color.GREEN, 10)) {
            System.err.println("Top-right quadrant failed: expected green, got " + trColor);
            return false;
        }
        
        // Check bottom-left (should be blue)
        Color blColor = getPixelColor(buffer, 10, 10, width, stride);
        if (!isApproximately(blColor, Color.BLUE, 10)) {
            System.err.println("Bottom-left quadrant failed: expected blue, got " + blColor);
            return false;
        }
        
        // Check bottom-right (should be yellow)
        Color brColor = getPixelColor(buffer, width - 10, 10, width, stride);
        if (!isApproximately(brColor, Color.YELLOW, 10)) {
            System.err.println("Bottom-right quadrant failed: expected yellow, got " + brColor);
            return false;
        }
        
        return true;
    }
    
    private static Color getPixelColor(ByteBuffer buf, int x, int y, 
                                       int width, int stride) {
        int offset = y * stride + x * 4;
        int b = Byte.toUnsignedInt(buf.get(offset + 0));
        int g = Byte.toUnsignedInt(buf.get(offset + 1));
        int r = Byte.toUnsignedInt(buf.get(offset + 2));
        int a = Byte.toUnsignedInt(buf.get(offset + 3));
        return Color.rgb(r, g, b, a / 255.0);
    }
    
    private static boolean isApproximately(Color c1, Color c2, int tolerance) {
        int r1 = (int)(c1.getRed() * 255);
        int g1 = (int)(c1.getGreen() * 255);
        int b1 = (int)(c1.getBlue() * 255);
        
        int r2 = (int)(c2.getRed() * 255);
        int g2 = (int)(c2.getGreen() * 255);
        int b2 = (int)(c2.getBlue() * 255);
        
        return Math.abs(r1 - r2) <= tolerance &&
               Math.abs(g1 - g2) <= tolerance &&
               Math.abs(b1 - b2) <= tolerance;
    }
}
```

---

## 8. Usage Examples

### 8.1 Enabling Diagnostics in Application

```java
// In AstraeusApp or WorkspaceWindow

// Add menu item or keyboard shortcut
MenuItem diagMenuItem = new MenuItem("Toggle Diagnostics (F3)");
diagMenuItem.setOnAction(e -> toggleDiagnostics());

KeyCodeCombination f3 = new KeyCodeCombination(KeyCode.F3);
scene.getAccelerators().put(f3, this::toggleDiagnostics);

private void toggleDiagnostics() {
    diagnosticsEnabled = !diagnosticsEnabled;
    viewport.enableDiagnostics(diagnosticsEnabled);
    
    if (diagnosticsEnabled) {
        System.out.println("Diagnostics enabled (F3 to toggle)");
    }
}
```

### 8.2 Automated Testing with Diagnostics

```java
@Test
public void testResizePreservesCameraAspect() {
    try (NativeEngine engine = new NativeEngine(800, 600, true)) {
        engine.setDiagnosticMode(true);
        engine.setValidationLevel(2);
        
        // Render a frame
        engine.beginFrame(0.016);
        engine.endFrame();
        
        // Check diagnostics
        FrameDiagnosticsJava diag = engine.getFrameDiagnostics();
        assertFalse(diag.hasValidationFlag(ValidationFlag.CAMERA_ASPECT_WRONG),
                    "Initial camera aspect should be correct");
        
        // Resize
        engine.resizeViewport(1280, 720);
        
        // Render another frame
        engine.beginFrame(0.016);
        engine.endFrame();
        
        // Check diagnostics again
        diag = engine.getFrameDiagnostics();
        assertFalse(diag.hasValidationFlag(ValidationFlag.CAMERA_ASPECT_WRONG),
                    "Camera aspect should be updated after resize");
    }
}
```

---

## 9. Performance Considerations

### 9.1 Overhead Analysis

**Diagnostic Mode Disabled (Release)**:
- Overhead: 0% (code compiled out)

**Diagnostic Mode Enabled, Level 1 (Debug)**:
- Per-frame snapshot copy: ~50 μs
- Basic validation checks: ~20 μs  
- Total overhead: ~70 μs (~0.4% at 60 FPS)

**Diagnostic Mode Enabled, Level 3 (Paranoid)**:
- All Level 1 checks: ~70 μs
- Per-entity validation: ~5 μs per entity
- For 1000 entities: ~5 ms (~30% at 60 FPS)
- **Only use Level 3 for debugging specific issues**

### 9.2 Optimization Strategies

1. **Conditional Compilation**: Wrap diagnostic code in `#ifdef ASTRAEUS_ENABLE_DIAGNOSTICS`
2. **Lazy Evaluation**: Only compute expensive checks when diagnostic overlay is visible
3. **Sampling**: Validate 10% of entities per frame instead of all
4. **Caching**: Cache validation results that don't change every frame

---

## 10. Summary

The Diagnostics Framework provides:

✅ **Unified per-frame snapshot** with all visualization pipeline state  
✅ **Real-time validation** of correctness invariants  
✅ **Visual diagnostic overlay** with color-coded warnings  
✅ **Test pattern rendering** for isolating presentation issues  
✅ **Automated verification** for regression testing  
✅ **Minimal overhead** when disabled or at basic level  

**Next Steps**:
1. Implement FrameDiagnostics struct and API in C++
2. Add validation logic for all correctness invariants
3. Create Java wrapper and diagnostic overlay UI
4. Implement test pattern rendering
5. Add automated test cases with verification
