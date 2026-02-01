# Visualization Correctness Testing Guide

**Document**: VIS-GEN-001 Testing & Regression  
**Purpose**: Define testing procedures and regression prevention  
**Scope**: Automated tests, manual validation, and CI integration

---

## 1. Overview

This guide provides comprehensive testing procedures to validate visualization correctness and prevent regressions. Tests are organized by layer and concern.

### 1.1 Testing Pyramid

```
        ┌─────────────────┐
        │   Manual E2E    │  ← Visual validation, exploratory testing
        └─────────────────┘
             ↑
        ┌─────────────────┐
        │  Integration    │  ← Cross-layer tests (Native ↔ Java)
        └─────────────────┘
             ↑
        ┌─────────────────┐
        │  Unit Tests     │  ← Per-component validation
        └─────────────────┘
```

---

## 2. Unit Tests

### 2.1 ABI Struct Layout Tests (C++)

**File**: `engine/tests/abi_struct_test.cpp`

```cpp
#include <gtest/gtest.h>
#include "api/EngineAPI.h"

TEST(ABIStructs, PixelBufferView_SizeAndAlignment) {
    // Verify size
    EXPECT_EQ(sizeof(PixelBufferView), 40);
    
    // Verify alignment
    EXPECT_EQ(alignof(PixelBufferView), 8);
}

TEST(ABIStructs, PixelBufferView_FieldOffsets) {
    EXPECT_EQ(offsetof(PixelBufferView, data), 0);
    EXPECT_EQ(offsetof(PixelBufferView, width), 8);
    EXPECT_EQ(offsetof(PixelBufferView, height), 12);
    EXPECT_EQ(offsetof(PixelBufferView, stride), 16);
    EXPECT_EQ(offsetof(PixelBufferView, format), 20);
    EXPECT_EQ(offsetof(PixelBufferView, max_backing_width), 24);
    EXPECT_EQ(offsetof(PixelBufferView, max_backing_height), 28);
    EXPECT_EQ(offsetof(PixelBufferView, max_backing_size), 32);
}

TEST(ABIStructs, FrameStats_SizeAndAlignment) {
    EXPECT_EQ(sizeof(FrameStats), 40);
    EXPECT_EQ(alignof(FrameStats), 8);
}

TEST(ABIStructs, CameraDesc_SizeAndAlignment) {
    // May be 56 or 64 depending on padding
    EXPECT_GE(sizeof(CameraDesc), 56);
    EXPECT_LE(sizeof(CameraDesc), 64);
}
```

### 2.2 ABI Struct Layout Tests (Java)

**File**: `java/frontend/src/test/java/com/astraeus/native_api/layout/ABILayoutTest.java`

```java
@Test
public void testPixelBufferViewLayout() {
    MemoryLayout layout = PixelBufferViewLayout.LAYOUT;
    
    // Verify size
    assertEquals(40, layout.byteSize());
    
    // Verify alignment
    assertEquals(8, layout.byteAlignment());
    
    // Verify field offsets
    assertEquals(0, layout.byteOffset(PathElement.groupElement("data")));
    assertEquals(8, layout.byteOffset(PathElement.groupElement("width")));
    assertEquals(12, layout.byteOffset(PathElement.groupElement("height")));
    assertEquals(16, layout.byteOffset(PathElement.groupElement("stride")));
    assertEquals(20, layout.byteOffset(PathElement.groupElement("format")));
}

@Test
public void testFrameStatsLayout() {
    MemoryLayout layout = FrameStatsLayout.LAYOUT;
    
    assertEquals(40, layout.byteSize());
    assertEquals(8, layout.byteAlignment());
}
```

### 2.3 Buffer Validation Tests (C++)

**File**: `engine/tests/buffer_validation_test.cpp`

```cpp
TEST(BufferValidation, StrideMinimum) {
    uint32_t width = 1920;
    uint32_t bpp = 4; // BGRA8
    uint32_t min_stride = width * bpp;
    
    // Stride must be at least width * bpp
    EXPECT_GE(min_stride, 7680);
}

TEST(BufferValidation, ViewportWithinBacking) {
    uint32_t backing_w = 2560;
    uint32_t backing_h = 1440;
    
    uint32_t viewport_w = 1920;
    uint32_t viewport_h = 1080;
    
    EXPECT_LE(viewport_w, backing_w);
    EXPECT_LE(viewport_h, backing_h);
}

TEST(BufferValidation, ReadbackSizeCalculation) {
    uint32_t width = 1920;
    uint32_t height = 1080;
    uint32_t stride = 1920 * 4; // Tightly packed
    
    uint32_t expected_size = height * stride;
    EXPECT_EQ(expected_size, 1920 * 1080 * 4);
}
```

---

## 3. Integration Tests

### 3.1 ABI Compatibility Test (Java)

**File**: `java/frontend/src/test/java/com/astraeus/integration/ABICompatibilityTest.java`

```java
@Test
public void testABICompatibilityAtStartup() {
    // Create engine
    NativeEngine engine = new NativeEngine(800, 600, true);
    
    try {
        // Get ABI info from native side
        ABIInfo info = engine.getABIInfo();
        
        // Verify struct sizes match
        assertEquals(40, info.getSizeofPixelBufferView());
        assertEquals(40, info.getSizeofFrameStats());
        
        // Verify pointer size
        assertEquals(8, info.getPointerSize());
        
        // Verify platform
        assertNotNull(info.getPlatformName());
        System.out.println("Platform: " + info.getPlatformName());
        
    } finally {
        engine.close();
    }
}
```

### 3.2 Resize Integration Test

**File**: `java/frontend/src/test/java/com/astraeus/integration/ResizeIntegrationTest.java`

```java
@Test
public void testResizePreservesCameraAspect() {
    NativeEngine engine = new NativeEngine(800, 600, true);
    NativeViewport viewport = engine.createViewport(800, 600);
    
    try {
        // Enable diagnostics
        engine.setDiagnosticMode(true);
        
        // Render initial frame
        engine.beginFrame(0.016);
        engine.endFrame();
        
        // Check initial camera aspect
        FrameDiagnosticsJava diag = engine.getFrameDiagnostics();
        float initialAspect = (float) diag.getViewportWidth() / diag.getViewportHeight();
        assertEquals(initialAspect, diag.getCameraAspect(), 0.01f);
        
        // Resize to 16:9
        viewport.resizeWithProjection(1920, 1080, 60.0f, 0.1f, 1000.0f);
        
        // Render frame
        engine.beginFrame(0.016);
        engine.endFrame();
        
        // Check camera aspect updated
        diag = engine.getFrameDiagnostics();
        float newAspect = (float) diag.getViewportWidth() / diag.getViewportHeight();
        assertEquals(newAspect, diag.getCameraAspect(), 0.01f);
        
        // Verify no validation errors
        assertEquals(0, diag.getValidationFlags());
        
    } finally {
        viewport.close();
        engine.close();
    }
}

@Test
public void testResizeClampingToMaxDimensions() {
    NativeEngine engine = new NativeEngine(800, 600, true);
    NativeViewport viewport = engine.createViewport(800, 600);
    
    try {
        // Configure backing buffer max size
        engine.configureReadback(2560, 1440, false);
        
        // Try to resize beyond max (should clamp)
        viewport.resizeWithProjection(3840, 2160, 60.0f, 0.1f, 1000.0f);
        
        // Render frame
        engine.beginFrame(0.016);
        engine.endFrame();
        
        // Verify clamped to max
        FrameDiagnosticsJava diag = engine.getFrameDiagnostics();
        assertTrue(diag.getViewportWidth() <= 2560);
        assertTrue(diag.getViewportHeight() <= 1440);
        
    } finally {
        viewport.close();
        engine.close();
    }
}
```

### 3.3 Entity Visibility Test

**File**: `java/frontend/src/test/java/com/astraeus/integration/EntityVisibilityTest.java`

```java
@Test
public void testEntityCreationMakesVisible() {
    NativeEngine engine = new NativeEngine(800, 600, true);
    
    try {
        // Enable diagnostics
        engine.setDiagnosticMode(true);
        
        // Initial state: no entities
        engine.beginFrame(0.016);
        engine.endFrame();
        
        FrameDiagnosticsJava diag = engine.getFrameDiagnostics();
        assertEquals(0, diag.getTotalEntities());
        assertEquals(0, diag.getVisibleEntities());
        
        // Create entity
        int entityId = engine.createEntity();
        
        // Set as renderable (visible)
        engine.setEntityRenderable(entityId, true);
        
        // Render frame
        engine.beginFrame(0.016);
        engine.endFrame();
        
        // Verify entity is visible
        diag = engine.getFrameDiagnostics();
        assertEquals(1, diag.getTotalEntities());
        assertEquals(1, diag.getVisibleEntities());
        
        // Hide entity
        engine.setEntityRenderable(entityId, false);
        
        // Render frame
        engine.beginFrame(0.016);
        engine.endFrame();
        
        // Verify entity is hidden
        diag = engine.getFrameDiagnostics();
        assertEquals(1, diag.getTotalEntities());
        assertEquals(0, diag.getVisibleEntities());
        
    } finally {
        engine.close();
    }
}
```

### 3.4 Test Pattern Validation

**File**: `java/frontend/src/test/java/com/astraeus/integration/TestPatternValidationTest.java`

```java
@Test
public void testQuadrantPatternCorrectness() {
    NativeEngine engine = new NativeEngine(1024, 768, true);
    
    try {
        // Enable test pattern
        engine.setTestPattern(TestPatternType.QUADRANTS);
        
        // Render frame
        engine.beginFrame(0.016);
        engine.endFrame();
        
        // Get color buffer
        PixelBufferView view = engine.getColorBuffer();
        ByteBuffer buffer = view.getDataAsBuffer();
        
        // Verify quadrant pattern
        boolean valid = TestPatternVerifier.verifyQuadrants(
            buffer, view.width(), view.height(), view.stride());
        
        assertTrue(valid, "Quadrant test pattern should be correct");
        
        // Disable test pattern
        engine.setTestPattern(TestPatternType.NONE);
        
    } finally {
        engine.close();
    }
}
```

---

## 4. Stress Tests

### 4.1 Rapid Resize Stress Test

**File**: `java/frontend/src/test/java/com/astraeus/stress/ResizeStressTest.java`

```java
@Test
public void testRapidResizing() {
    NativeEngine engine = new NativeEngine(800, 600, true);
    NativeViewport viewport = engine.createViewport(800, 600);
    engine.setDiagnosticMode(true);
    
    try {
        int[][] sizes = {
            {800, 600},
            {1280, 720},
            {1920, 1080},
            {1024, 768},
            {1600, 900},
            {2560, 1440},
            {1280, 720},
            {800, 600}
        };
        
        for (int[] size : sizes) {
            // Resize
            viewport.resizeWithProjection(size[0], size[1], 60.0f, 0.1f, 1000.0f);
            
            // Render multiple frames
            for (int i = 0; i < 5; i++) {
                engine.beginFrame(0.016);
                engine.endFrame();
                
                // Validate each frame
                FrameDiagnosticsJava diag = engine.getFrameDiagnostics();
                
                // Check no validation errors
                assertEquals(0, diag.getValidationFlags(), 
                    "Validation errors at size " + size[0] + "x" + size[1]);
                
                // Check camera aspect
                float expectedAspect = (float) size[0] / size[1];
                assertEquals(expectedAspect, diag.getCameraAspect(), 0.01f,
                    "Camera aspect wrong at size " + size[0] + "x" + size[1]);
            }
        }
        
    } finally {
        viewport.close();
        engine.close();
    }
}
```

### 4.2 Entity Churn Test

**File**: `java/frontend/src/test/java/com/astraeus/stress/EntityChurnTest.java`

```java
@Test
public void testEntityCreateDeleteChurn() {
    NativeEngine engine = new NativeEngine(800, 600, true);
    engine.setDiagnosticMode(true);
    
    try {
        for (int iteration = 0; iteration < 100; iteration++) {
            List<Integer> entities = new ArrayList<>();
            
            // Create 50 entities
            for (int i = 0; i < 50; i++) {
                int id = engine.createEntity();
                engine.setEntityRenderable(id, true);
                entities.add(id);
            }
            
            // Render frame
            engine.beginFrame(0.016);
            engine.endFrame();
            
            // Validate counts
            FrameDiagnosticsJava diag = engine.getFrameDiagnostics();
            assertEquals(50, diag.getTotalEntities());
            assertEquals(50, diag.getVisibleEntities());
            
            // Delete all entities
            for (int id : entities) {
                engine.destroyEntity(id);
            }
            
            // Render frame
            engine.beginFrame(0.016);
            engine.endFrame();
            
            // Validate empty
            diag = engine.getFrameDiagnostics();
            assertEquals(0, diag.getTotalEntities());
            assertEquals(0, diag.getVisibleEntities());
        }
        
    } finally {
        engine.close();
    }
}
```

---

## 5. Manual Validation

### 5.1 Visual Inspection Checklist

**Setup**:
1. Launch AstraeusApp
2. Enable diagnostics (F3 key)
3. Enable test pattern mode (menu or hotkey)

**Tests**:

- [ ] **Quadrant Test Pattern**
  - Verify TL=Red, TR=Green, BL=Blue, BR=Yellow
  - Check sharp boundaries at center
  - No color bleeding or blending

- [ ] **Resize Window**
  - Drag window edge to resize
  - Verify no visual distortion (stretched/squashed)
  - Check diagnostic overlay shows matching VP/camera aspect
  - Verify test pattern remains correct

- [ ] **Maximize/Minimize**
  - Maximize window
  - Verify full screen rendering
  - Minimize and restore
  - Check no artifacts

- [ ] **Camera Operations**
  - Orbit camera with mouse drag
  - Verify camera affects full viewport
  - Zoom in/out
  - Check no clipping issues

- [ ] **Entity Creation**
  - Create entity (button click)
  - Verify entity appears in 3D view
  - Verify entity count in diagnostic overlay increments
  - Delete entity
  - Verify entity disappears

- [ ] **Rapid Operations**
  - Rapidly resize window multiple times
  - Rapidly create/delete entities
  - Rapidly orbit/zoom camera
  - Check no crashes or visual corruption

### 5.2 Test Pattern Verification

**Quadrants (Red/Green/Blue/Yellow)**:
```java
engine.setTestPattern(TestPatternType.QUADRANTS);
```
- [ ] Top-left quadrant is pure red
- [ ] Top-right quadrant is pure green
- [ ] Bottom-left quadrant is pure blue
- [ ] Bottom-right quadrant is pure yellow
- [ ] Boundaries are sharp (no gradients)

**Checkerboard**:
```java
engine.setTestPattern(TestPatternType.CHECKERBOARD);
```
- [ ] Alternating black/white squares
- [ ] Squares are perfectly aligned
- [ ] No drift or skew
- [ ] Squares remain square (not rectangular)

**Gradient**:
```java
engine.setTestPattern(TestPatternType.GRADIENT);
```
- [ ] Smooth horizontal red-to-green gradient
- [ ] No banding or discontinuities
- [ ] Gradient fills entire viewport

**Grid**:
```java
engine.setTestPattern(TestPatternType.GRID);
```
- [ ] Red vertical lines
- [ ] Green horizontal lines
- [ ] Lines are straight (no wobble)
- [ ] Lines evenly spaced

---

## 6. Regression Prevention

### 6.1 Pre-Commit Checks

Before committing changes that affect visualization:

```bash
# Run ABI tests
cd build
ctest -R abi_test

# Run Java integration tests
cd java/frontend
mvn test -Dtest=ABICompatibilityTest
mvn test -Dtest=ResizeIntegrationTest
mvn test -Dtest=EntityVisibilityTest
```

### 6.2 CI Pipeline

Add to `.github/workflows/ci.yml`:

```yaml
name: Visualization Correctness

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup C++ Build
        run: |
          mkdir build && cd build
          cmake ..
          cmake --build .
      
      - name: Run C++ ABI Tests
        run: |
          cd build
          ctest --output-on-failure -R abi_test
      
      - name: Setup Java
        uses: actions/setup-java@v3
        with:
          java-version: '21'
          distribution: 'temurin'
      
      - name: Run Java Tests
        run: |
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/build/lib
          cd java/frontend
          mvn test
      
      - name: Upload Test Results
        if: always()
        uses: actions/upload-artifact@v3
        with:
          name: test-results
          path: '**/target/surefire-reports/*.xml'
```

### 6.3 Nightly Stress Tests

Add to `.github/workflows/nightly.yml`:

```yaml
name: Nightly Stress Tests

on:
  schedule:
    - cron: '0 2 * * *'  # Run at 2 AM daily

jobs:
  stress-test:
    runs-on: ubuntu-latest
    
    steps:
      - uses: actions/checkout@v3
      
      # ... setup steps ...
      
      - name: Run Resize Stress Test
        run: |
          cd java/frontend
          mvn test -Dtest=ResizeStressTest
      
      - name: Run Entity Churn Test
        run: |
          mvn test -Dtest=EntityChurnTest
      
      - name: Notify on Failure
        if: failure()
        uses: slackapi/slack-github-action@v1
        with:
          webhook-url: ${{ secrets.SLACK_WEBHOOK }}
          payload: |
            {
              "text": "Nightly stress tests failed!"
            }
```

---

## 7. Debugging Failed Tests

### 7.1 ABI Mismatch

**Symptom**: `testABICompatibilityAtStartup()` fails with size mismatch

**Debug Steps**:
1. Print native struct size: `sizeof(PixelBufferView)`
2. Print Java layout size: `PixelBufferViewLayout.LAYOUT.byteSize()`
3. Compare field offsets with `offsetof()` and `layout.byteOffset()`
4. Check for missing padding in schema

**Fix**: Update `abi_structs_schema.yaml` and regenerate layouts

### 7.2 Camera Aspect Mismatch

**Symptom**: `testResizePreservesCameraAspect()` fails

**Debug Steps**:
1. Enable diagnostic mode
2. Print viewport dimensions
3. Print camera aspect ratio
4. Check if `resizeWithProjection()` was called
5. Verify camera projection update code path

**Fix**: Ensure atomic resize + camera update, check calculation

### 7.3 Entity Not Visible

**Symptom**: `testEntityCreationMakesVisible()` fails

**Debug Steps**:
1. Check if `setEntityRenderable(id, true)` was called
2. Verify World has entity in `renderable_entities_cache_`
3. Check render pass iteration logic
4. Enable pass logging to see submitted entities

**Fix**: Call `syncVisibilityToEngine()` after entity creation

---

## 8. Performance Testing

### 8.1 Frame Time Budget

**Target**: 16.67ms per frame (60 FPS)

**Breakdown**:
- Engine begin/end: <0.5ms
- Render passes: <12ms
- Readback: <1ms
- JavaFX display: <3ms

### 8.2 Diagnostic Overhead

**Level 0 (None)**: 0% overhead (compiled out)  
**Level 1 (Basic)**: <0.5% overhead (~70μs)  
**Level 2 (Full)**: <2% overhead (~300μs)  
**Level 3 (Paranoid)**: 10-30% overhead (use for debugging only)

### 8.3 Memory Footprint

**Backing Buffers** (2560x1440x4 each):
- Color buffer: 14.75 MB
- ID buffer: 14.75 MB
- Total: ~30 MB (fixed, never grows)

**Java Heap** (typical):
- NativeEngine: <1 MB
- Scene entities (1000): ~1 MB
- UI components: ~5 MB
- Total: <10 MB (excluding JavaFX framework)

---

## 9. Test Coverage Goals

### 9.1 Unit Test Coverage

- [ ] ABI struct layouts: 100%
- [ ] Buffer validation: 100%
- [ ] Field offset validation: 100%

### 9.2 Integration Test Coverage

- [ ] Resize flow: 100%
- [ ] Camera updates: 100%
- [ ] Entity visibility: 100%
- [ ] Test pattern rendering: 100%

### 9.3 Manual Test Coverage

- [ ] All test patterns validated visually
- [ ] All resize scenarios tested
- [ ] All camera modes tested
- [ ] Entity CRUD operations tested

---

## 10. Summary

**Testing Strategy**:

✅ **Unit tests** for struct layouts and low-level validation  
✅ **Integration tests** for cross-layer correctness  
✅ **Stress tests** for stability under load  
✅ **Manual tests** for visual validation  
✅ **CI integration** for automated regression prevention  

**Key Tests**:

1. **ABI Compatibility** - Verify struct layouts match
2. **Resize Integration** - Ensure camera aspect stays correct
3. **Entity Visibility** - Validate scene-to-render pipeline
4. **Test Patterns** - Verify buffer integrity and presentation

**Regression Prevention**:

- Pre-commit: Run critical tests locally
- CI: Run full test suite on every push
- Nightly: Run stress tests for stability
- Manual: Perform visual validation before release

**Next Steps**:

1. Implement test infrastructure (test harness, fixtures)
2. Write all unit and integration tests
3. Set up CI pipeline
4. Document manual testing procedures
5. Run full test suite and fix any failures
