# VIS-001: Testing and Verification Guide

This guide explains how to test and verify the JavaFX PixelBuffer split-view corruption fix.

## Quick Start

### Enable Debug Logging
```bash
java -Dastraeus.debug.bufferUpdate=true \
     -Dastraeus.debug.testPattern=true \
     -jar AstraeusApp.jar
```

### Test Patterns
The application includes built-in test patterns to verify correct rendering:

```java
// In your code or via console:
viewport.setTestPattern(FxViewport.TestPattern.GRADIENT);
```

Available patterns:
- `NONE` - Normal rendering (default)
- `GRADIENT` - Red→Green horizontal gradient (tests continuity)
- `CHECKERBOARD` - Alternating black/white squares (tests alignment)
- `GRID` - Red vertical + green horizontal lines (tests stride)
- `COLOR_BANDS` - Horizontal color stripes (tests row order)
- `QUADRANTS` - Four colored quadrants (tests frame boundaries)

## Manual Test Scenarios

### Test 1: Small Viewport (Stride Mode)
Tests stride-aware copying when viewport < backing buffer.

**Setup:**
1. Start application with max backing: 2560×1440
2. Set viewport to: 1280×720

**Expected Behavior:**
- Console shows: "Viewport smaller than backing buffer - using stride-aware copy"
- `useStrideCopy = true`
- Test patterns display correctly without artifacts
- No split-view or corruption

**Verification:**
```bash
# Check console output for:
[FxViewport] PixelBuffer Configuration:
  Backing dimensions: 2560x1440
  Viewport dimensions: 1280x720
  Stride: 10240 bytes/row
  ...
Viewport smaller than backing buffer - using stride-aware copy
Created tightly-packed PixelBuffer: 1280x720
```

### Test 2: Full Size Viewport (Direct Mode)
Tests direct mode when viewport == backing buffer.

**Setup:**
1. Start application with max backing: 2560×1440
2. Set viewport to: 2560×1440

**Expected Behavior:**
- Console shows: "Viewport matches backing buffer - direct buffer usage"
- `useStrideCopy = false`
- Zero overhead, no copying
- Test patterns display correctly

**Verification:**
```bash
# Check console output for:
[FxViewport] PixelBuffer Configuration:
  Backing dimensions: 2560x1440
  Viewport dimensions: 2560x1440
  ...
Viewport matches backing buffer - direct buffer usage
```

### Test 3: Dynamic Resize
Tests PixelBuffer recreation during runtime resize.

**Steps:**
1. Start at 1280×720 (stride mode)
2. Resize to 1920×1080 (still stride mode)
3. Resize to 2560×1440 (switches to direct mode)
4. Resize back to 1280×720 (switches to stride mode)

**Expected Behavior:**
- Smooth transition between sizes
- No artifacts during or after resize
- Console shows "Recreating PixelBuffer" messages
- Test patterns remain correct at all sizes

**Verification:**
```bash
# Check console for resize messages:
[FxViewport] Resizing viewport: 1280x720 -> 1920x1080
[FxViewport] After resize:
  Viewport: 1920x1080
  Stride: 10240 bytes/row
  Backing: 2560x1440
[FxViewport] Recreating PixelBuffer for new viewport size: 1920x1080
PixelBuffer recreated successfully
```

### Test 4: Stride Validation
Tests that stride values are correct and validated.

**Setup:**
1. Enable debug logging: `-Dastraeus.debug.bufferUpdate=true`
2. Run with any viewport size < backing size

**Expected Behavior:**
- No stride mismatch warnings (unless native has issues)
- Stride equals max_backing_width × 4
- Buffer capacity matches max_backing_size

**Verification:**
```bash
# Should see (once per second when debug enabled):
[FxViewport] copyWithStride:
  Viewport: 1280x720
  Stride: 10240 bytes/row
  Row bytes: 5120

# Should NOT see:
[NativeEngine] WARNING: Stride mismatch in color buffer!
```

## Visual Verification

### Gradient Pattern
**Purpose:** Verify horizontal and vertical continuity

**What to Look For:**
- Smooth left-to-right transition from red to green
- Smooth top-to-bottom transition (red at top, green at bottom)
- No discontinuities, breaks, or repeated sections
- No vertical "split" where colors don't match

**Bad:** 
```
RED  |  GREEN  ← Split here = stride issue
     |  GREEN
```

**Good:**
```
RED → YELLOW → GREEN  ← Smooth transition
RED → YELLOW → GREEN
```

### Checkerboard Pattern
**Purpose:** Verify pixel alignment and stride handling

**What to Look For:**
- Perfect alternating black and white squares
- No shifted or misaligned rows
- No half-squares or broken patterns
- Consistent square size throughout

**Bad:**
```
▪▫▪▫▪
▫▪▫▪▫
▪ ▫▪▫  ← Row shifted = alignment issue
```

**Good:**
```
▪▫▪▫▪
▫▪▫▪▫
▪▫▪▫▪
```

### Grid Pattern
**Purpose:** Verify row and column alignment

**What to Look For:**
- Straight red vertical lines (no breaks or offsets)
- Straight green horizontal lines (no breaks or offsets)
- Lines intersect at perfect right angles
- No diagonal sections or stair-stepping

**Bad:**
```
| | |
--+--+--
| |  |  ← Vertical line offset = stride issue
--+--+--
```

**Good:**
```
| | |
--+--+--
| | |
--+--+--
```

### Color Bands
**Purpose:** Verify row order and completeness

**What to Look For:**
- Horizontal bands of solid colors
- Consistent band height across the width
- No bands that are wider on one side
- Colors cycle: Blue, Green, Red, Cyan, Magenta, Yellow, White, Gray

**Bad:**
```
[BLUE BLUE BLUE]
[GREEN    GREEN]  ← Incomplete band = stride issue
[RED RED RED]
```

**Good:**
```
[BLUE BLUE BLUE]
[GREEN GREEN GREEN]
[RED RED RED]
```

### Quadrants
**Purpose:** Verify frame boundaries and orientation

**What to Look For:**
- Four solid color quadrants meeting at the center
- Sharp boundary lines (not blurred or overlapped)
- Correct colors:
  - Top-left: Red
  - Top-right: Green
  - Bottom-left: Blue
  - Bottom-right: Yellow

**Bad:**
```
[RED   ][GREEN]
[RED   ][BLUE ]  ← Wrong color placement
```

**Good:**
```
[RED   ][GREEN]
[BLUE  ][YELLOW]
```

## Performance Verification

### Frame Time Measurement
Monitor copy time to ensure acceptable performance.

**Method 1: Java Instrumentation**
```java
long start = System.nanoTime();
viewport.updateDisplay();
long elapsed = System.nanoTime() - start;
System.out.println("Copy time: " + (elapsed / 1_000_000.0) + " ms");
```

**Expected Results:**
- 1280×720: < 0.5 ms
- 1920×1080: < 0.8 ms
- 2560×1440 (direct): < 0.01 ms (no copy)

**Method 2: FPS Monitoring**
Enable telemetry overlay (F2) and watch FPS:
- Should maintain 60 FPS at common resolutions
- Small drop (55-60 FPS) acceptable at very large sizes with stride copy

## Automated Testing (TODO)

Future automated tests to implement:

```java
@Test
public void testStrideDetection() {
    FxViewport viewport = new FxViewport(engine, 2560, 1440, 1280, 720);
    assertTrue(viewport.usesStrideCopy());
}

@Test
public void testDirectMode() {
    FxViewport viewport = new FxViewport(engine, 1920, 1080, 1920, 1080);
    assertFalse(viewport.usesStrideCopy());
}

@Test
public void testResizeRecreatesBuffer() {
    FxViewport viewport = new FxViewport(engine, 2560, 1440, 1280, 720);
    PixelBuffer original = viewport.getPixelBuffer();
    viewport.resizeViewport(1920, 1080);
    PixelBuffer after = viewport.getPixelBuffer();
    assertNotSame(original, after);
}

@Test
public void testTestPatternGeneration() {
    ByteBuffer buffer = ByteBuffer.allocateDirect(1280 * 720 * 4);
    TestPatternGenerator.fillGradient(buffer, 1280, 720, 1280 * 4);
    // Verify first pixel is red, last pixel is green
    buffer.position(0);
    assertEquals((byte)0, buffer.get()); // B
    assertEquals((byte)0, buffer.get()); // G
    assertEquals((byte)255, buffer.get()); // R
}
```

## Troubleshooting

### Problem: Split-view still visible
**Possible Causes:**
1. Stride copy not enabled (check `useStrideCopy` flag)
2. Native stride calculation incorrect
3. Buffer state corruption (position/limit not reset)

**Debug Steps:**
```bash
# Enable all debugging
java -Dastraeus.debug.assertBufferState=true \
     -Dastraeus.debug.bufferUpdate=true \
     -jar app.jar

# Check console for:
# 1. "using stride-aware copy" message
# 2. No "ByteBuffer state corrupted" warnings
# 3. Stride value matches expectation (max_width * 4)
```

### Problem: Performance issues
**Possible Causes:**
1. Stride copy overhead too high
2. Excessive logging enabled
3. Large viewport size

**Debug Steps:**
- Measure copy time (see Performance Verification)
- Disable debug logging
- Try smaller viewport or increase backing buffer match

### Problem: Test patterns show artifacts
**Interpretation:**
- Gradient breaks: Horizontal stride issue
- Checkerboard shift: Row alignment issue
- Grid stair-stepping: Stride calculation error
- Bands incomplete: Partial row copy issue

**Fix:**
Check stride calculation in both Java and native code.

## Sign-Off Checklist

Before marking VIS-001 as complete, verify:

- [ ] Test pattern `GRADIENT` displays smoothly with no breaks
- [ ] Test pattern `CHECKERBOARD` shows perfect alignment
- [ ] Test pattern `GRID` has straight lines with no offsets
- [ ] Test pattern `COLOR_BANDS` shows complete horizontal bands
- [ ] Test pattern `QUADRANTS` shows four solid color regions
- [ ] Viewport at 1280×720 works correctly (stride mode)
- [ ] Viewport at 1920×1080 works correctly (stride mode)
- [ ] Viewport at 2560×1440 works correctly (direct mode)
- [ ] Dynamic resize works smoothly without artifacts
- [ ] Console shows correct stride and buffer dimensions
- [ ] No buffer state corruption warnings
- [ ] Frame time is acceptable (< 1ms for stride copy)
- [ ] FPS remains stable (> 55 FPS)

## Summary

The fix implements stride-aware pixel copying to resolve JavaFX PixelBuffer limitations. The test patterns provide visual confirmation that the data layout is correct, while debug logging provides technical validation of buffer configuration and performance.

If all test patterns display correctly and console logs show expected values, the fix is working properly.
