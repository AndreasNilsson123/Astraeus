# Visualization Correctness Invariants

**Document**: VIS-GEN-001 Correctness Audit  
**Purpose**: Define hard invariants and contracts for the visualization pipeline  
**Scope**: Native (C++) → FFM/ABI → Java (JavaFX) integration

---

## 1. Core Invariants

### 1.1 Buffer Size Invariants

**INV-BUF-001**: Fixed Backing Buffer Size
```
nativeBackingBufferBytes == maxWidth * maxHeight * bytesPerPixel
```
- The backing buffer is allocated once at engine initialization
- Pointer address MUST remain stable for engine lifetime
- No reallocation on viewport resize

**INV-BUF-002**: Viewport Within Backing Buffer
```
viewportWidth <= maxBackingWidth
viewportHeight <= maxBackingHeight
viewportBytes <= backingBufferBytes
```
- Current viewport dimensions MUST NOT exceed backing buffer dimensions
- Resize operations that exceed max dimensions MUST be clamped or rejected

**INV-BUF-003**: Stride Alignment
```
stride >= width * bytesPerPixel
stride % alignment == 0  (typically 4-byte or 16-byte alignment)
```
- Stride MUST be at least width * bytesPerPixel
- Stride MUST respect platform alignment requirements
- For tightly-packed buffers: `stride == width * bytesPerPixel`

**INV-BUF-004**: Buffer Readback Consistency
```
readbackBytes == viewportWidth * viewportHeight * bytesPerPixel (for tightly-packed)
readbackBytes == viewportHeight * stride (for strided buffers)
```
- Readback size MUST match viewport dimensions and format
- Partial reads MUST NOT occur (no torn frames)

### 1.2 Pixel Format Invariants

**INV-FMT-001**: Explicit Format Contract
```
colorBufferFormat == PIXEL_FORMAT_BGRA8 (or explicitly specified)
idBufferFormat == PIXEL_FORMAT_R32UI
```
- Pixel format MUST be explicitly documented and verified
- BGRA8 vs RGBA8 ambiguity MUST be resolved at ABI boundary
- JavaFX expects BGRA8 on most platforms (Windows/Linux)

**INV-FMT-002**: Bytes Per Pixel
```
BGRA8: bytesPerPixel == 4
R32UI: bytesPerPixel == 4
```
- Bytes per pixel MUST match format specification
- Mismatch causes visual corruption or crashes

### 1.3 Coordinate System Invariants

**INV-COORD-001**: Device Pixel Mapping
```
nativeViewportPx == javaDevicePx
```
- Native framebuffer dimensions MUST match Java device pixels
- DPI scaling MUST be handled consistently
- Logical pixels vs device pixels MUST be clearly distinguished

**INV-COORD-002**: Viewport Origin
```
viewportOrigin == (0, 0) at bottom-left (OpenGL) or top-left (Framebuffer readback)
```
- Y-axis orientation MUST be documented
- Picking coordinates MUST be transformed correctly
- Camera projection MUST match coordinate system

### 1.4 Resize Invariants

**INV-RESIZE-001**: Deterministic Resize Order
```
setViewportSize(w, h) → updateCameraProjection(aspect) → render() → readback() → present()
```
- Resize operations MUST follow strict ordering
- Camera aspect ratio MUST be updated before next frame
- No partial-frame camera updates allowed

**INV-RESIZE-002**: No Torn Frames on Resize
```
frame(n).viewport == frame(n).camera.aspect == frame(n).readback.dimensions
```
- All frame components MUST use consistent dimensions
- Stale viewport/scissor state MUST be cleared on resize

**INV-RESIZE-003**: Resize Clamping
```
requestedWidth > maxWidth → clamp to maxWidth (log warning)
requestedHeight > maxHeight → clamp to maxHeight (log warning)
```
- Resize requests exceeding max dimensions MUST be handled gracefully
- No silent failures or undefined behavior

---

## 2. ABI/FFM Contracts

### 2.1 Struct Layout Contracts

**ABI-LAYOUT-001**: Fixed Struct Sizes
```
sizeof(PixelBufferView) == 32 bytes (x64, standard alignment)
sizeof(FrameStats) == 40 bytes
sizeof(CameraDesc) == 64 bytes
```
- Struct sizes MUST be validated at compile time (C++) and runtime (Java)
- Padding MUST be explicit in schema
- Cross-platform consistency MUST be verified

**ABI-LAYOUT-002**: Field Alignment
```
alignof(pointer) == 8 (x64)
alignof(uint64) == 8
alignof(float64) == 8
alignof(uint32) == 4
```
- Field alignment MUST match platform ABI
- Padding MUST prevent misaligned access
- Generated layouts MUST match native layouts exactly

**ABI-LAYOUT-003**: Padding Validation
```
Every struct MUST have explicit padding for 8-byte alignment (x64)
Padding fields MUST be named _padding or _paddingN
```
- No implicit padding allowed
- All padding MUST be documented in schema

### 2.2 Pointer Stability Contracts

**ABI-PTR-001**: Stable Buffer Pointers
```
PixelBufferView.data address MUST remain constant after allocation
JavaFX MemorySegment MUST wrap stable pointer
```
- No pointer invalidation during engine lifetime
- Double-buffer mode may swap pointers but each buffer is stable

**ABI-PTR-002**: Null Pointer Safety
```
All API functions MUST validate pointer parameters
NULL pointers MUST return error codes, not crash
```
- Defensive programming at ABI boundary
- Error codes MUST be checked on Java side

### 2.3 Memory Ownership Contracts

**ABI-OWN-001**: Native Ownership
```
C++ owns all GPU buffers, framebuffers, and readback memory
Java MUST NOT allocate or resize native buffers
Java MUST NOT modify native buffer pointers
```
- Clear ownership boundaries
- Java reads only, native writes

**ABI-OWN-002**: Lifecycle Management
```
Buffer lifetime == Engine lifetime (or until explicit destroy)
Java MemorySegment MUST NOT outlive native buffer
Arena scoping MUST match native lifecycle
```
- Explicit lifecycle contracts
- AutoCloseable pattern for safety

---

## 3. Scene-to-Render Parity Invariants

### 3.1 Entity Visibility Invariants

**SCENE-VIS-001**: Renderable Entity Synchronization
```
entity.visible == true → entity IN renderable_entities_cache
entity.visible == false → entity NOT IN renderable_entities_cache
```
- Visibility state MUST be synchronized immediately
- No stale visibility state in render passes

**SCENE-VIS-002**: Transform Propagation
```
setEntityTransform(id, T) → World.transforms[id] == T
nextFrame: renderSubmission.transforms[id] == T
```
- Transform updates MUST be visible in next frame
- No lost or delayed transform updates

**SCENE-VIS-003**: Entity Count Consistency
```
World.entity_count == sum(all entity maps)
FrameStats.entity_count == World.entity_count
Java.entities.size() == World.entity_count (eventually consistent)
```
- Entity counts MUST be consistent across subsystems
- Diagnostic overlays MUST reflect true counts

### 3.2 Render Submission Invariants

**SCENE-RENDER-001**: No Silent Filtering
```
entity.visible && entity.has_renderable → entity MUST be submitted to passes
```
- No silent culling without explicit reason (frustum, layer mask, etc.)
- All visible entities MUST reach appropriate render passes

**SCENE-RENDER-002**: NaN/Inf Handling
```
entity.transform contains NaN/Inf → log warning, skip rendering (don't crash)
```
- Invalid transforms MUST be handled gracefully
- Diagnostic logging for invalid data

**SCENE-RENDER-003**: Layer Mask Consistency
```
entity.layer_mask & pass.layer_mask != 0 → entity is candidate for pass
```
- Layer mask filtering MUST be explicit and documented
- Default layer masks MUST allow rendering

---

## 4. Camera Propagation Invariants

### 4.1 Projection Matrix Invariants

**CAM-PROJ-001**: Aspect Ratio from Viewport
```
camera.aspect == viewport.width / viewport.height
```
- Aspect ratio MUST be derived from current viewport dimensions
- Resize MUST trigger camera aspect update

**CAM-PROJ-002**: FOV Consistency
```
camera.fov_degrees MUST be in valid range [1.0, 179.0]
camera.near_plane > 0
camera.far_plane > near_plane
```
- Invalid camera parameters MUST be rejected or clamped
- No degenerate projection matrices

**CAM-PROJ-003**: Viewport-Scissor Sync
```
glViewport(0, 0, viewport.width, viewport.height)
glScissor(0, 0, viewport.width, viewport.height)
```
- Viewport and scissor MUST match current dimensions
- Stale viewport state MUST be cleared on resize

### 4.2 Camera Update Timing

**CAM-UPDATE-001**: Camera Before Render
```
resize(w, h) → update_camera_projection() → render_frame()
```
- Camera updates MUST complete before rendering
- No partial-frame camera application

**CAM-UPDATE-002**: Deterministic Frame State
```
frame(n).camera == state at beginFrame(n)
frame(n).camera remains constant until beginFrame(n+1)
```
- Camera state MUST be immutable during frame rendering
- No mid-frame camera changes

---

## 5. Diagnostic & Observability Invariants

### 5.1 Debug Information

**DIAG-INFO-001**: Unified Frame Snapshot
```
FrameDiagnostics contains:
  - viewport dimensions (logical, device, native)
  - pixel formats and byte counts
  - stride and alignment info
  - entity counts (total, visible, submitted)
  - frame index and timing
```
- All diagnostic info accessible from single struct
- Updated every frame

**DIAG-INFO-002**: Size Mismatch Detection
```
IF (native.width != java.deviceWidth) → log ERROR, show overlay warning
IF (readback.bytes != expected.bytes) → log ERROR, abort frame
```
- Mismatches MUST be detected and logged immediately
- Visual indicators for diagnostic mode

### 5.2 Test Pattern Validation

**DIAG-TEST-001**: Toggleable Test Pattern
```
test_pattern_mode == true → render test pattern instead of scene
test_pattern includes: quadrants (R/G/B/Y), gradient, checkerboard
```
- Test patterns isolate presentation issues from scene issues
- Patterns MUST be pixel-perfect and verifiable

**DIAG-TEST-002**: Test Pattern Verification
```
Java can query test pattern mode
Java can verify expected pixel values in test pattern
```
- Automated verification of test patterns
- Regression testing for buffer integrity

---

## 6. Error Handling Contracts

### 6.1 Graceful Degradation

**ERR-HANDLE-001**: No Crashes on Invalid Input
```
Invalid parameters → return error code, log warning
NULL pointers → return error code, don't dereference
Out of range → clamp or reject, don't overflow
```
- All API functions MUST be defensive
- Never crash on bad input from Java

**ERR-HANDLE-002**: Error Propagation
```
Native error → AstraeusResult error code → Java exception (if appropriate)
Error message MUST be descriptive and actionable
```
- Errors MUST propagate clearly
- Error messages include context

### 6.2 Recovery Strategies

**ERR-RECOVER-001**: Frame Skip on Error
```
render_frame() fails → skip frame, log error, continue
readback() fails → skip frame, log error, continue
```
- Single frame errors MUST NOT crash application
- Errors logged for debugging

---

## 7. Threading Model Contracts

### 7.1 Single-Threaded Assumptions

**THREAD-001**: UI Thread Only
```
All engine API calls MUST come from JavaFX Application Thread
No concurrent access from multiple threads
```
- Current design is single-threaded
- No synchronization overhead

**THREAD-002**: Future Multi-Threading Preparation
```
API design MUST allow future render thread separation
Opaque handles MUST be thread-safe IDs (not raw pointers)
```
- Design for future concurrency
- Handles facilitate thread safety

---

## 8. Verification Checklist

### 8.1 Pre-Flight Checks (Engine Init)

- [ ] Verify struct sizes match between C++ and Java
- [ ] Validate backing buffer allocation succeeded
- [ ] Check pointer addresses are non-NULL
- [ ] Verify pixel format matches expectations
- [ ] Validate max dimensions are reasonable

### 8.2 Per-Frame Checks (Debug Mode)

- [ ] Viewport dimensions within max bounds
- [ ] Readback size matches viewport * bytesPerPixel
- [ ] No stale viewport/scissor state
- [ ] Camera aspect ratio matches viewport
- [ ] Entity count consistency across subsystems

### 8.3 Resize Validation

- [ ] Camera aspect ratio updated before next frame
- [ ] Viewport and scissor match new dimensions
- [ ] No visual artifacts (tearing, pixelation)
- [ ] Backing buffer pointer unchanged
- [ ] Debug overlay shows correct dimensions

### 8.4 ABI/FFM Validation

- [ ] Struct layouts match (size and offset)
- [ ] Pointer alignment correct
- [ ] No buffer overruns on Java side
- [ ] MemorySegment bounds correct
- [ ] No UnsatisfiedLinkError or access violations

---

## 9. Test Patterns for Validation

### 9.1 Quadrant Test
- **Purpose**: Verify frame boundaries and orientation
- **Pattern**: TL=Red, TR=Green, BL=Blue, BR=Yellow
- **Expected**: Sharp boundaries at midpoints, no color bleeding

### 9.2 Checkerboard Test
- **Purpose**: Verify stride and alignment
- **Pattern**: Alternating black/white squares (32x32 pixels)
- **Expected**: Perfect square grid, no drift or skew

### 9.3 Gradient Test
- **Purpose**: Verify horizontal/vertical continuity
- **Pattern**: Smooth red-to-green horizontal gradient
- **Expected**: No banding, tearing, or discontinuities

### 9.4 Grid Test
- **Purpose**: Verify row/column alignment
- **Pattern**: Red vertical lines, green horizontal lines
- **Expected**: Straight lines, no wobble or offset

---

## 10. Regression Prevention

### 10.1 Automated Tests

**REG-TEST-001**: Resize Stress Test
```
for size in [(800,600), (1280,720), (1920,1080), (2560,1440)]:
    resize(size)
    render()
    verify_no_artifacts()
    verify_camera_aspect()
```

**REG-TEST-002**: Camera Operation Test
```
resize(1280, 720)
orbit_camera()
verify_camera_affects_full_viewport()
resize(1920, 1080)
verify_camera_still_correct()
```

**REG-TEST-003**: Entity Visibility Test
```
entity = create_entity()
verify_entity_visible_in_frame()
set_entity_visible(false)
verify_entity_not_rendered()
```

### 10.2 Manual Validation

- [ ] Run application with test pattern mode
- [ ] Perform multiple resizes while rotating camera
- [ ] Create/delete entities and verify visibility
- [ ] Check debug overlay for consistent metrics
- [ ] Verify no console errors or warnings

---

## 11. Documentation Requirements

### 11.1 ABI Documentation

- [ ] Document all struct layouts with sizes/offsets
- [ ] Specify supported platforms and alignment
- [ ] Explain pointer stability guarantees
- [ ] List all error codes and meanings

### 11.2 Integration Guide

- [ ] Document resize best practices
- [ ] Explain buffer lifetime and ownership
- [ ] Provide JavaFX PixelBuffer integration example
- [ ] List common pitfalls and solutions

### 11.3 Diagnostic Guide

- [ ] Explain debug overlay metrics
- [ ] Document test pattern usage
- [ ] Provide troubleshooting flowchart
- [ ] List validation tools and scripts

---

## Summary

This document defines the hard invariants and contracts for the Astraeus visualization pipeline. All invariants MUST be upheld for correct visualization. Any violation MUST be treated as a bug and fixed immediately.

**Key Principles**:
1. **Explicit is better than implicit** - all contracts documented
2. **Fail fast and loud** - detect violations immediately
3. **Observable and debuggable** - provide diagnostic tools
4. **Stable and predictable** - no surprising behavior
5. **Testable and verifiable** - automated regression prevention
