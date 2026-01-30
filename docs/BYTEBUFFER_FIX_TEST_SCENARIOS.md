# ByteBuffer State Fix - Test Scenarios

This document outlines test scenarios to validate the ByteBuffer state corruption fix.

## Prerequisites

Ensure the application is built and ready to run:
```bash
./build.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/build/lib
```

## Test Scenario 1: Extended Rendering (1000+ frames)

**Purpose**: Verify no state corruption occurs during normal continuous rendering.

**Steps**:
1. Launch the application
2. Let it render for at least 1000 frames (approx 16 seconds at 60fps)
3. Observe console output and JavaFX display

**Expected Results**:
- ✅ No exceptions thrown
- ✅ No "BaseTexture.checkUpdateParams" errors
- ✅ No "Upload requires ... elements" errors
- ✅ Smooth continuous rendering
- ✅ No visual artifacts or corruption

**Pass/Fail**: ___________

---

## Test Scenario 2: Dynamic Viewport Resizing

**Purpose**: Verify buffer state remains stable during viewport dimension changes.

**Steps**:
1. Launch the application with initial viewport size 1280x720
2. Resize viewport to 1920x1080
3. Resize back to 800x600
4. Resize to 2560x1440 (if within max bounds)
5. Resize to 640x480
6. Let render for 100 frames at each size

**Expected Results**:
- ✅ No exceptions during or after any resize
- ✅ Viewport content updates correctly at each size
- ✅ No buffer state warnings if debug assertions enabled
- ✅ Performance remains stable

**Pass/Fail**: ___________

---

## Test Scenario 3: Debug Assertions Enabled

**Purpose**: Validate that the buffer state invariants hold under all conditions.

**Steps**:
1. Launch with debug flag:
   ```bash
   java -Dastraeus.debug.assertBufferState=true -jar ...
   ```
2. Run through Scenarios 1 and 2 above
3. Monitor console for any warnings

**Expected Results**:
- ✅ No warnings about buffer state corruption
- ✅ No position != 0 warnings
- ✅ No limit != capacity warnings
- ✅ All buffer state checks pass

**Example of PASS (no warnings)**:
```
[FxViewportV2] Created viewport with controller and overlays
[NativeEngine] Readback configured: 2560x1440 (double_buffer=false)
... normal operation, no warnings ...
```

**Example of FAIL (would show warnings)**:
```
[FxViewportV2] WARNING: ByteBuffer state corrupted! position=123 limit=4608000 capacity=14745600
[FxViewportV2] Expected: position=0 limit=capacity=14745600
```

**Pass/Fail**: ___________

---

## Test Scenario 4: Rapid Resize Stress Test

**Purpose**: Stress test the buffer state with rapid viewport changes.

**Steps**:
1. Create a test loop that resizes viewport every frame
2. Alternate between multiple sizes: 1280x720 → 800x600 → 1920x1080 → 640x480
3. Run for 100 resize cycles
4. Monitor for exceptions or state warnings

**Expected Results**:
- ✅ No exceptions during rapid resizing
- ✅ No state corruption warnings
- ✅ Application remains stable
- ✅ No memory leaks or growing memory usage

**Pass/Fail**: ___________

---

## Test Scenario 5: Multiple Viewports (if supported)

**Purpose**: Verify buffer state isolation across multiple viewport instances.

**Steps**:
1. Create 2-3 viewport instances with different sizes
2. Render to all viewports simultaneously
3. Resize one viewport while others continue rendering
4. Close one viewport, continue rendering others

**Expected Results**:
- ✅ Each viewport maintains stable buffer state
- ✅ No cross-contamination between viewports
- ✅ No exceptions when one viewport is resized or closed
- ✅ Other viewports unaffected

**Pass/Fail**: ___________

---

## Test Scenario 6: Camera Movement + Rendering

**Purpose**: Verify buffer state remains stable during interactive use.

**Steps**:
1. Launch application
2. Use camera controls (orbit/pan/zoom)
3. Perform entity picking (click on objects)
4. Enable telemetry overlay (F2 key)
5. Continue camera movement with overlays active

**Expected Results**:
- ✅ No buffer state issues during camera movement
- ✅ Picking works correctly
- ✅ Overlays render without corruption
- ✅ No exceptions during interactive use

**Pass/Fail**: ___________

---

## Debugging Failed Tests

If any test fails, collect the following information:

1. **Console Output**: Full console log including stack traces
2. **JVM Arguments**: Note any special JVM flags used
3. **System Info**: Java version, OS, JavaFX version
4. **Exception Details**: Full exception message and stack trace
5. **Reproduction Steps**: Exact sequence that triggers the issue

### Common Issues and Fixes

**Issue**: "BaseTexture.checkUpdateParams" exception still occurs
- **Possible Cause**: Code path mutating buffer state wasn't found
- **Debug**: Enable assertions and check for warnings
- **Fix**: Search for other buffer.position() or buffer.limit() calls

**Issue**: Visual artifacts or corruption
- **Possible Cause**: Native memory being written at wrong offset
- **Debug**: Verify stride and viewport calculations
- **Fix**: Check C++ readback implementation

**Issue**: Memory leak or growing memory
- **Possible Cause**: ByteBuffer duplicates not being GC'd
- **Debug**: Use profiler to check buffer allocations
- **Fix**: Ensure duplicates are short-lived, not cached

---

## Automated Test Suite (Future Work)

Recommended automated tests to add:

```java
@Test
public void testBufferStateStability() {
    NativeEngine engine = new NativeEngine(1920, 1080, false);
    engine.configureReadback(2560, 1440, false);
    
    PixelBufferView view = engine.getColorBuffer();
    ByteBuffer buffer = view.getByteBuffer();
    
    // Buffer should be stable
    assertEquals(0, buffer.position());
    assertEquals(buffer.capacity(), buffer.limit());
    
    // Get another view - buffer state should not change
    view = engine.getColorBuffer();
    buffer = view.getByteBuffer();
    
    assertEquals(0, buffer.position());
    assertEquals(buffer.capacity(), buffer.limit());
    
    engine.close();
}

@Test
public void testViewportBufferDuplicate() {
    NativeEngine engine = new NativeEngine(1920, 1080, false);
    engine.configureReadback(2560, 1440, false);
    
    PixelBufferView view = engine.getColorBuffer();
    ByteBuffer stable = view.getByteBuffer();
    ByteBuffer sized = view.getViewportBuffer();
    
    // Buffers share native memory but have independent state
    assertNotSame(stable, sized);
    assertEquals(0, stable.position());
    assertEquals(stable.capacity(), stable.limit());
    
    // Sized buffer has viewport limit
    assertEquals(0, sized.position());
    assertTrue(sized.limit() <= stable.capacity());
    
    // Mutating sized buffer should not affect stable buffer
    sized.position(10);
    assertEquals(0, stable.position());
    
    engine.close();
}
```

---

## Test Sign-off

| Test Scenario | Pass/Fail | Tester | Date | Notes |
|--------------|-----------|--------|------|-------|
| 1. Extended Rendering | | | | |
| 2. Dynamic Resizing | | | | |
| 3. Debug Assertions | | | | |
| 4. Rapid Resize Stress | | | | |
| 5. Multiple Viewports | | | | |
| 6. Interactive Use | | | | |

**Overall Status**: ___________  
**Signed off by**: ___________  
**Date**: ___________
