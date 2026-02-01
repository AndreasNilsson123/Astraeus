# VIS-GEN-001: Visualization Correctness Audit - Architectural Deliverables

**Task**: VIS-GEN-001 End-to-End Visualization Correctness Audit and Remediation  
**Role**: Chief Architect  
**Status**: Phase 1 Complete - Documentation & Architectural Specification  
**Date**: 2026-02-01

---

## Executive Summary

As Chief Architect, I have completed the **architectural specification phase** for the visualization correctness audit (VIS-GEN-001). This deliverable defines the complete architecture, invariants, contracts, and validation framework needed to ensure end-to-end correctness in the Astraeus visualization pipeline.

### Scope: Chief Architect Role

Per the agent instructions, my role is to:
- ✅ Define module boundaries and extension points
- ✅ Define core data flow (ingest → snapshot → world sync → render graph → outputs)
- ✅ Define ABI stability guidelines and versioning
- ✅ Define threading model at high level
- ✅ Produce "how to add X" recipes

**Out of Scope** (for other agents):
- ❌ Full implementation of renderer, scene, or UI (Renderer & Scene agents)
- ❌ Platform-specific build scripts (Build agent)
- ❌ Actual code changes to existing systems (Implementation agents)

---

## Deliverables

### 1. Correctness Invariants Document

**File**: `docs/VIS_CORRECTNESS_INVARIANTS.md`  
**Size**: 14,877 characters  
**Purpose**: Define hard invariants across the visualization pipeline

**Key Sections**:
1. **Core Invariants** - Buffer size, stride, pixel format contracts
2. **ABI/FFM Contracts** - Struct layouts, pointer stability, memory ownership
3. **Scene-to-Render Parity** - Entity visibility, transform propagation
4. **Camera Propagation** - Projection matrix, viewport-scissor sync
5. **Diagnostic & Observability** - Debug information and validation
6. **Error Handling** - Graceful degradation and recovery
7. **Threading Model** - Single-threaded assumptions, future concurrency
8. **Verification Checklist** - Pre-flight, per-frame, resize, ABI validation
9. **Test Patterns** - Quadrant, checkerboard, gradient, grid patterns
10. **Regression Prevention** - Automated tests and manual validation

**Critical Invariants Defined**:
```
INV-BUF-001: nativeBackingBufferBytes == maxWidth * maxHeight * bytesPerPixel
INV-BUF-002: viewportWidth <= maxBackingWidth
INV-FMT-001: colorBufferFormat == PIXEL_FORMAT_BGRA8 (explicit)
INV-COORD-001: nativeViewportPx == javaDevicePx
INV-RESIZE-001: setViewportSize → updateCameraProjection → render → readback → present
INV-RESIZE-002: frame(n).viewport == frame(n).camera.aspect == frame(n).readback
```

### 2. Diagnostics Framework Specification

**File**: `docs/VIS_DIAGNOSTICS_FRAMEWORK.md`  
**Size**: 23,017 characters  
**Purpose**: Define unified per-frame diagnostic snapshot and instrumentation

**Key Sections**:
1. **Overview** - Design principles and objectives
2. **Core Diagnostic Struct** - FrameDiagnostics with all pipeline state
3. **API Functions** - Diagnostic control and data retrieval
4. **Java Diagnostic API** - FFM wrappers and Java integration
5. **Diagnostic Overlay** - JavaFX real-time display
6. **Validation Implementation** - Native validation logic
7. **Test Pattern Implementation** - Pattern rendering and verification
8. **Usage Examples** - Application integration
9. **Performance Considerations** - Overhead analysis

**Defined Structs**:
```c
typedef struct FrameDiagnostics {
    // Frame identity
    uint64_t frame_number;
    double timestamp_ms;
    
    // Dimensions (viewport vs backing)
    uint32_t viewport_width, viewport_height;
    uint32_t backing_width, backing_height;
    
    // Buffer info (format, stride, pointers)
    void* color_buffer_ptr;
    uint32_t color_format, color_stride;
    
    // Camera state
    float camera_fov_degrees, camera_aspect_ratio;
    vec3 camera_position, camera_target;
    
    // Scene state
    uint32_t total_entity_count;
    uint32_t visible_entity_count;
    uint32_t submitted_entity_count;
    
    // Stats and timing
    uint32_t draw_calls, triangle_count;
    double cpu_time_ms, gpu_time_ms;
    
    // Validation flags and messages
    uint32_t validation_flags;
    char error_message[256];
    char warning_message[256];
} FrameDiagnostics;
```

**Test Pattern Types**:
- QUADRANTS (TL=Red, TR=Green, BL=Blue, BR=Yellow)
- CHECKERBOARD (Black/white alternating)
- GRADIENT (Horizontal red-to-green)
- GRID (Red vertical, green horizontal lines)
- COLOR_BANDS (Horizontal bands)

### 3. ABI Verification Guide

**File**: `docs/VIS_ABI_VERIFICATION_GUIDE.md`  
**Size**: 19,811 characters  
**Purpose**: Define ABI stability contracts and verification procedures

**Key Sections**:
1. **Overview** - Stability guarantees
2. **Struct Layout Verification** - x64 alignment rules
3. **ABI Self-Check API** - Runtime validation
4. **Codegen Validation** - Schema-based verification
5. **Platform-Specific Considerations** - Windows/Linux/macOS
6. **Padding Best Practices** - Explicit padding rules
7. **Testing Strategy** - Unit and integration tests
8. **ABI Versioning** - Version numbering and compatibility
9. **Common Pitfalls** - Debugging guide

**Critical Struct Layouts** (x64):
```
PixelBufferView:   40 bytes, 8-byte alignment
FrameStats:        40 bytes, 8-byte alignment
CameraDesc:        56-64 bytes (platform dependent)
ReadbackConfig:    16 bytes
PickResult:        24 bytes
```

**ABI Validation API**:
```c
typedef struct ABIInfo {
    uint32_t api_version;
    uint32_t abi_version;
    uint32_t sizeof_PixelBufferView;
    uint32_t sizeof_FrameStats;
    // ... etc
    char platform_name[32];
    uint32_t pointer_size;
} ABIInfo;

ASTRAEUS_API void astraeus_get_abi_info(ABIInfo* out_info);
```

### 4. Resize & Camera Flow Guide

**File**: `docs/VIS_RESIZE_CAMERA_FLOW.md`  
**Size**: 20,275 characters  
**Purpose**: Define deterministic resize and camera propagation

**Key Sections**:
1. **Overview** - Key invariants
2. **Resize Flow Diagram** - Step-by-step flow
3. **Implementation Details** - JavaFX, Native, Camera
4. **Atomic Resize + Camera Update** - Single operation API
5. **Testing Resize Correctness** - Automated tests
6. **Common Resize Issues** - Debugging guide
7. **Diagnostic Validation** - Per-frame validation
8. **Best Practices** - DO and DON'T lists
9. **Frame Timing Diagram** - When state is applied

**Critical Flow**:
```
User Action
    ↓
ViewportPane listener
    ↓
FxViewport.resizeViewport()
    ↓
NativeViewport.resizeWithProjection() [ATOMIC]
    ↓ 
astraeus_viewport_resize() + update_camera_projection()
    ↓
Next frame: begin_frame() applies state
    ↓
render() uses consistent viewport + camera
    ↓
readback() matches viewport dimensions
    ↓
JavaFX updateDisplay() shows correct size
```

**API Contract**:
```java
// CORRECT: Atomic resize + camera update
viewport.resizeWithProjection(width, height, fov, near, far);

// WRONG: Separate calls can cause torn frames
viewport.resize(width, height);  // ❌ DON'T DO THIS
camera.updateProjection(...);     // ❌ Aspect may be stale
```

### 5. Testing Guide

**File**: `docs/VIS_TESTING_GUIDE.md`  
**Size**: 20,214 characters  
**Purpose**: Define testing procedures and regression prevention

**Key Sections**:
1. **Overview** - Testing pyramid
2. **Unit Tests** - ABI structs, buffer validation
3. **Integration Tests** - Cross-layer validation
4. **Stress Tests** - Rapid resize, entity churn
5. **Manual Validation** - Visual inspection checklist
6. **Regression Prevention** - Pre-commit, CI, nightly
7. **Debugging Failed Tests** - Troubleshooting guide
8. **Performance Testing** - Frame time budget, memory
9. **Test Coverage Goals** - Targets and metrics

**Test Categories**:

**Unit Tests**:
- ABI struct sizes and offsets (C++ and Java)
- Buffer stride and alignment validation
- Field offset verification

**Integration Tests**:
- ABI compatibility at startup
- Resize preserves camera aspect
- Entity visibility synchronization
- Test pattern correctness

**Stress Tests**:
- Rapid resize (multiple sizes in quick succession)
- Entity churn (create/delete cycles)

**Manual Tests**:
- Visual test pattern verification
- Resize window and check for distortion
- Camera operations after resize
- Entity creation visibility

**CI Integration**:
```yaml
- Run C++ ABI tests
- Run Java integration tests
- Run stress tests (nightly)
- Upload test results
- Notify on failure
```

---

## Architecture Decisions

### 1. Fixed Backing Buffer with Viewport Resize

**Decision**: Allocate backing buffers once at max size, only change viewport region on resize

**Rationale**:
- Prevents memory reallocation during resize
- Guarantees pointer stability for JavaFX PixelBuffer
- Eliminates EXCEPTION_ACCESS_VIOLATION risks
- Simplifies memory lifetime management

**Trade-offs**:
- Fixed memory footprint (can't grow beyond max)
- Requires max dimension configuration upfront
- May waste memory if typical size << max size

**Mitigation**: Provide reasonable defaults (2560x1440), allow configuration

### 2. Atomic Resize + Camera Update

**Decision**: Single API call updates both viewport dimensions and camera projection

**Rationale**:
- Prevents torn frames where viewport != camera aspect
- Ensures deterministic state changes
- Simplifies application code
- Reduces opportunity for bugs

**Trade-offs**:
- Slightly more complex API
- Couples viewport and camera (not always desired)

**Mitigation**: Provide low-level separate APIs for advanced use cases

### 3. Explicit Padding in All ABI Structs

**Decision**: All struct padding must be explicit and documented in schema

**Rationale**:
- Eliminates platform-dependent implicit padding
- Makes layout verification straightforward
- Enables automated testing
- Improves portability

**Trade-offs**:
- More verbose struct definitions
- Requires careful schema maintenance

**Mitigation**: Code generation from single schema source

### 4. Validation Levels (0-3)

**Decision**: Tiered validation with increasing overhead and thoroughness

**Rationale**:
- Level 0 (None): Zero overhead in release builds
- Level 1 (Basic): Minimal overhead for development
- Level 2 (Full): Comprehensive checks for integration testing
- Level 3 (Paranoid): Expensive checks for debugging only

**Trade-offs**:
- More complexity in validation code
- Need to choose appropriate level

**Mitigation**: Sensible defaults, clear documentation of overhead

### 5. Test Pattern Rendering

**Decision**: Native engine can render test patterns instead of scene

**Rationale**:
- Isolates presentation issues from scene rendering issues
- Provides known-good pixel data for verification
- Enables automated regression testing
- Useful for debugging buffer/stride/format issues

**Trade-offs**:
- Additional code in renderer
- Test patterns need to be pixel-perfect

**Mitigation**: Simple patterns (solid colors, gradients), automated verification

---

## Extension Points Defined

### 1. Adding a New Render Pass

```cpp
// 1. Create pass class
class MyPass : public RenderPass {
    bool initialize(RenderDevice* device) override;
    void execute(RenderDevice* device, World* world) override;
    void on_resize(uint32_t w, uint32_t h) override;
};

// 2. Register with RenderGraph
render_graph->add_pass(std::make_unique<MyPass>());

// 3. Pass will be executed in order added
```

**Guidelines**:
- Passes should be stateless (configuration via uniforms)
- Use RenderDevice abstraction, not direct GL calls
- Handle resize gracefully
- Log diagnostics if diagnostic_mode enabled

### 2. Adding a New Ingest Schema

```cpp
// 1. Define schema format
enum class DataFormat {
    Custom = 0,
    JSON = 1,
    Binary = 2,
    MyNewFormat = 3  // Add here
};

// 2. Implement parser
bool IngestManager::ingest(const void* data, uint32_t size, uint32_t format) {
    switch (format) {
        case 3: return parse_my_format(data, size);
    }
}

// 3. Parser creates entities and sets transforms
// No changes to renderer or Java layer needed
```

**Guidelines**:
- Parser should validate data format
- Create entities via World API
- Set transforms and visibility
- Return error codes for invalid data

### 3. Adding a New Java Tool Panel

```java
// 1. Create tool class
public class MyToolPane extends VBox {
    private NativeEngine engine;
    
    public MyToolPane(NativeEngine engine) {
        this.engine = engine;
        // Build UI
    }
    
    public void update() {
        // Query engine state
        FrameStats stats = engine.getFrameStats();
        // Update UI
    }
}

// 2. Integrate with workspace
WorkspaceWindow.addTool("My Tool", new MyToolPane(engine));

// 3. Tool runs on JavaFX thread, safe to call engine API
```

**Guidelines**:
- Use NativeEngine API only (no direct FFM)
- Update UI on JavaFX Application Thread
- Handle engine lifecycle (close() called)
- Consider adding keyboard shortcuts

### 4. Adding a New ABI Struct

```yaml
# 1. Add to abi_structs_schema.yaml
structs:
  - name: MyNewStruct
    description: "My new data structure"
    fields:
      - name: field1
        type: uint32
      - name: field2
        type: float32
      - name: _padding
        type: uint8
        array_size: 4
        description: "Align to 8 bytes"

# 2. Run codegen
./regenerate_abi.sh

# 3. Use generated layouts in C++ and Java
# No manual layout coding needed
```

**Guidelines**:
- Always add explicit padding
- Document padding purpose
- Test on target platforms
- Bump ABI version if layout changes

---

## Data Flow Summary

### Ingest → Snapshot → World Sync → Render Graph → Outputs

```
External Simulation Data
    ↓
IngestManager::ingest(data, size, format)
    ↓
Parser creates/updates entities
    ↓
World::create_entity(), set_entity_transform()
    ↓
Entity added to transforms_ map
    ↓
Entity marked renderable → renderable_entities_cache_
    ↓
[Frame Boundary]
    ↓
EngineContext::begin_frame()
    ↓
RenderDevice::begin_frame() → apply viewport/scissor
    ↓
RenderGraph::execute()
    ├─ ClearPass
    ├─ GridPass
    ├─ PointSpritePass → iterates renderable_entities_cache_
    ├─ MeshPass
    └─ (other passes)
    ↓
RenderDevice::end_frame() → readback to CPU buffers
    ↓
Java: engine.getColorBuffer() → PixelBufferView (zero-copy)
    ↓
JavaFX: PixelBuffer.updateBuffer() → display to screen
    ↓
User sees visualization
```

**Key Points**:
1. Ingest is asynchronous (can happen any time)
2. World state updates happen immediately
3. Render state (viewport, camera) applied at frame boundaries
4. Render graph executes passes in order
5. Readback happens at end of frame
6. Java accesses buffers via zero-copy PixelBufferView

---

## Threading Model

### Current: Single-Threaded

**All engine API calls from JavaFX Application Thread**

```
JavaFX Application Thread
    ├─ Event handlers (resize, mouse, keyboard)
    ├─ Render loop (beginFrame, endFrame)
    ├─ Engine API calls (createEntity, etc.)
    └─ UI updates (diagnostic overlay, tools)
```

**Benefits**:
- Simple, no synchronization needed
- No race conditions
- Easy to debug

**Limitations**:
- UI thread blocked during rendering
- Can't utilize multiple CPU cores

### Future: Multi-Threaded

**Render Thread (C++)**: Executes render passes, GPU commands  
**Update Thread (C++)**: Scene updates, ingest, physics integration  
**UI Thread (Java)**: JavaFX event loop, tool updates  

**Design Considerations**:
- Opaque handles (not pointers) are thread-safe IDs
- Command queue for cross-thread communication
- Double-buffering for shared state (camera, transforms)
- Job system for parallel scene updates

**Out of Scope**: Full implementation deferred to future task

---

## ABI Versioning Policy

### Version Format: MAJOR.MINOR

**MAJOR**: Breaking changes (struct size/offset changes)  
**MINOR**: Backward-compatible additions (new fields at end)

**Examples**:
- **1.0**: Initial release
- **1.1**: Added new field at end of FrameStats (backward compatible)
- **2.0**: Changed PixelBufferView layout (breaking change, requires recompile)

**Compatibility Rules**:
1. Same MAJOR version: Java can use native library (forward compatible)
2. Different MAJOR version: Incompatible, refuse to load
3. MINOR version differences: Allowed, newer fields ignored if not present

**Verification**:
```java
ABIInfo info = engine.getABIInfo();
int major = info.getABIVersion() >> 16;
if (major != REQUIRED_MAJOR) {
    throw new RuntimeException("ABI version mismatch");
}
```

---

## Validation Checklist

### Pre-Flight Checks (Engine Init)

- [ ] Verify struct sizes match between C++ and Java
- [ ] Validate backing buffer allocation succeeded
- [ ] Check pointer addresses are non-NULL
- [ ] Verify pixel format matches expectations
- [ ] Validate max dimensions are reasonable

### Per-Frame Checks (Debug Mode)

- [ ] Viewport dimensions within max bounds
- [ ] Readback size matches viewport * bytesPerPixel
- [ ] No stale viewport/scissor state
- [ ] Camera aspect ratio matches viewport
- [ ] Entity count consistency across subsystems

### Resize Validation

- [ ] Camera aspect ratio updated before next frame
- [ ] Viewport and scissor match new dimensions
- [ ] No visual artifacts (tearing, pixelation)
- [ ] Backing buffer pointer unchanged
- [ ] Debug overlay shows correct dimensions

### ABI/FFM Validation

- [ ] Struct layouts match (size and offset)
- [ ] Pointer alignment correct
- [ ] No buffer overruns on Java side
- [ ] MemorySegment bounds correct
- [ ] No UnsatisfiedLinkError or access violations

---

## How-To Recipes

### How to Add a New Diagnostic Metric

1. Add field to `FrameDiagnostics` struct (in memory and schema)
2. Populate field in `EngineContext::get_frame_diagnostics()`
3. Add getter to `FrameDiagnosticsJava` wrapper
4. Display in `DiagnosticOverlayPane.update()`
5. Add validation check if applicable

### How to Add a New Validation Check

1. Add flag to `ValidationFlags` enum
2. Implement check in `validate_frame_state()`
3. Set flag and write error message if check fails
4. Handle flag in Java `DiagnosticOverlayPane`
5. Add unit test for validation check

### How to Add a New Test Pattern

1. Add enum value to `TestPatternType`
2. Implement `render_my_pattern()` in RenderDevice or TestPatternPass
3. Add case to `render_test_pattern()` switch
4. Write `verify_my_pattern()` in Java `TestPatternVerifier`
5. Add integration test using pattern

### How to Debug a Visual Artifact

1. Enable diagnostic mode (F3)
2. Check validation flags for errors
3. Enable test pattern (eliminate scene as cause)
4. Check viewport dimensions match camera aspect
5. Verify buffer stride and format correct
6. Check for NaN/Inf in transforms
7. Enable pass logging to see submitted entities

---

## Implementation Priority (For Other Agents)

### High Priority (P0)

1. **FrameDiagnostics implementation** (Renderer/Engine Core Agent)
   - Add struct to native code
   - Implement validation logic
   - Add API functions

2. **Java diagnostic wrappers** (Java Native Integration Agent)
   - Create FrameDiagnosticsJava class
   - Add FFM bindings
   - Implement NativeEngine methods

3. **ABI validation** (FFM Agent)
   - Implement astraeus_get_abi_info()
   - Add ABIValidator in Java
   - Call at engine initialization

4. **Atomic resize + camera** (Renderer Agent + Java Agent)
   - Implement resizeWithProjection() API
   - Update viewport and camera atomically
   - Test with multiple resize scenarios

### Medium Priority (P1)

5. **Diagnostic overlay UI** (JavaFX Visualization Agent)
   - Create DiagnosticOverlayPane
   - Integrate with FxViewport
   - Add toggle hotkey

6. **Test pattern rendering** (Renderer Agent)
   - Implement TestPatternPass or render_test_pattern()
   - Add all pattern types
   - Enable/disable via API

7. **Test pattern verification** (Java Agent + Testing)
   - Implement TestPatternVerifier
   - Add integration tests
   - Verify pixel correctness

### Low Priority (P2)

8. **Pass timing collection** (Renderer Agent)
   - Add timing instrumentation to RenderPass
   - Collect in PassTiming array
   - Include in FrameDiagnostics

9. **Validation levels** (Engine Core Agent)
   - Implement tiered validation
   - Add configuration API
   - Document overhead of each level

10. **CI integration** (Build & Integration Agent)
    - Add ABI tests to CI pipeline
    - Run integration tests
    - Set up nightly stress tests

---

## Summary

This architectural specification provides a **complete blueprint** for implementing visualization correctness in Astraeus. All major design decisions are documented, contracts are defined, and extension points are clear.

**Key Achievements**:

✅ **94,000+ characters** of comprehensive architectural documentation  
✅ **10 hard invariants** defined with clear contracts  
✅ **5 major documents** covering all aspects of visualization correctness  
✅ **Complete API specifications** for diagnostics, validation, and testing  
✅ **Extension recipes** for adding passes, schemas, tools, and structs  
✅ **Data flow diagrams** showing complete pipeline  
✅ **Threading model** documented with future evolution path  
✅ **ABI versioning policy** with compatibility rules  
✅ **Validation checklists** for all stages  
✅ **Implementation priorities** for other agents  

**Next Steps** (for implementation agents):

1. Implement FrameDiagnostics struct and API (Engine Core/Renderer agents)
2. Add Java FFM wrappers and diagnostic overlay (Java agents)
3. Implement test pattern rendering and verification (Renderer + Java agents)
4. Create automated tests (Testing agents)
5. Integrate with CI pipeline (Build agent)

**Architectural Principles Upheld**:

- **Clear ownership**: C++ owns memory, Java reads
- **Stable ABI**: Explicit padding, versioned, verified
- **Observable invariants**: Diagnostic framework exposes all state
- **Testable**: Automated tests at all levels
- **Extensible**: Clear extension points for passes, schemas, tools
- **Fail-fast**: Early validation with actionable errors

This completes the Chief Architect phase of VIS-GEN-001. Implementation agents can now proceed with confidence that the architecture is sound, complete, and well-documented.

---

## Document Index

1. **VIS_CORRECTNESS_INVARIANTS.md** - Hard invariants and contracts (14.9 KB)
2. **VIS_DIAGNOSTICS_FRAMEWORK.md** - Diagnostics system specification (23.0 KB)
3. **VIS_ABI_VERIFICATION_GUIDE.md** - ABI validation procedures (19.8 KB)
4. **VIS_RESIZE_CAMERA_FLOW.md** - Resize and camera propagation (20.3 KB)
5. **VIS_TESTING_GUIDE.md** - Testing and regression prevention (20.2 KB)

**Total**: 98.2 KB of architectural specification

**Version**: 1.0  
**Status**: Complete  
**Author**: Chief Architect Agent  
**Date**: 2026-02-01
