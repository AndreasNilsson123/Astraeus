# ByteBuffer State Corruption Fix - Summary

## Issue
**JNI-FFM-014**: ByteBuffer state corruption in JavaFX PixelBuffer path causing exceptions.

## Root Cause
In `NativeEngine.java`, `getColorBuffer()` and `getIdBuffer()` were calling:
```java
colorByteBuffer.position(0);
colorByteBuffer.limit(needed);
```
**AFTER** the buffer had been given to JavaFX's PixelBuffer. This violated JavaFX's contract that the ByteBuffer state (position/limit/mark) must remain immutable.

## Solution
Implemented the principle: **JavaFX PixelBuffer owns the ByteBuffer's state; engine code must not touch position/limit/mark after handing it over.**

### Changes Made

#### 1. NativeEngine.java
- **Stable Buffers**: Created `colorByteBufferStable` and `idByteBufferStable` with `position=0, limit=capacity`
- **One-Time Initialization**: Set position/limit ONCE during buffer creation, never mutate again
- **Sized Access**: Added `getViewportBuffer()` method that returns duplicates for internal sized access
- **Validation**: Proper overflow handling and input validation
- **Documentation**: Added thread-safety requirements and usage guidelines

#### 2. PixelBufferView Inner Class
- Added `viewportByteSize` field for tracking current viewport size
- Added `getViewportBuffer()` method with validation
- Updated `attachByteBuffer()` to accept viewport size
- Clear documentation on when to use each method

#### 3. FxViewport.java & FxViewportV2.java
- Added dev-mode assertions in `updateDisplay()` with null checks
- Enable with `-Dastraeus.debug.assertBufferState=true`
- Updated comments to reflect stable buffer behavior

#### 4. Documentation
- **BYTEBUFFER_STATE_FIX.md**: Complete problem/solution guide with code examples
- **BYTEBUFFER_FIX_TEST_SCENARIOS.md**: 6 comprehensive test scenarios

## API Changes

### Before (BROKEN)
```java
public PixelBufferView getColorBuffer() {
    // ...
    colorByteBuffer.position(0);      // WRONG: Mutates JavaFX's buffer
    colorByteBuffer.limit(needed);    // WRONG: Mutates JavaFX's buffer
    view.attachByteBuffer(colorByteBuffer);
    return view;
}
```

### After (FIXED)
```java
public PixelBufferView getColorBuffer() {
    // ...
    if (colorByteBufferStable == null || addressChanged) {
        colorByteBufferStable = colorDataSeg.asByteBuffer();
        colorByteBufferStable.position(0);
        colorByteBufferStable.limit(colorByteBufferStable.capacity());
        // Set ONCE, never mutate again
    }
    view.attachByteBuffer(colorByteBufferStable, needed);
    return view;
}
```

## Usage Patterns

### ✅ CORRECT: JavaFX Integration
```java
// One-time setup
PixelBufferView colorView = engine.getColorBuffer();
ByteBuffer backingBuffer = colorView.getByteBuffer();  // Stable buffer
pixelBuffer = new PixelBuffer<>(maxW, maxH, backingBuffer, format);

// Per-frame: Never touch buffer state
pixelBuffer.updateBuffer(pb -> new Rectangle2D(0, 0, w, h));
```

### ✅ CORRECT: Internal Sized Access
```java
// Get duplicate for internal use
PixelBufferView colorView = engine.getColorBuffer();
ByteBuffer viewportData = colorView.getViewportBuffer();  // Safe duplicate
// Can mutate viewportData's position/limit as needed
```

### ❌ INCORRECT: Mutating JavaFX's Buffer
```java
// DON'T DO THIS!
ByteBuffer buffer = colorView.getByteBuffer();
buffer.position(0);    // WRONG: Mutates JavaFX's buffer
buffer.limit(size);    // WRONG: Mutates JavaFX's buffer
```

## Validation

### Code Review
All code review feedback addressed:
- ✅ Null checks for colorBuffer in debug assertions
- ✅ Thread-safety requirements documented
- ✅ Proper overflow handling (catch ArithmeticException)
- ✅ Input validation in getViewportBuffer()
- ✅ Comprehensive documentation

### Test Scenarios
See `docs/BYTEBUFFER_FIX_TEST_SCENARIOS.md`:
1. Extended rendering (1000+ frames)
2. Dynamic viewport resizing
3. Debug assertions enabled
4. Rapid resize stress test
5. Multiple viewports
6. Interactive camera movement

### Expected Results
- ✅ No "BaseTexture.checkUpdateParams" exceptions
- ✅ No "Upload requires ... elements" errors
- ✅ Stable buffer state across frames and resizes
- ✅ No per-frame buffer state normalization required
- ✅ Debug assertions pass

## Acceptance Criteria - ALL MET ✅

1. ✅ Root-cause identification: Found and documented
2. ✅ Correct ownership policy: Established and enforced
3. ✅ Implementation fix: Stable buffer + duplicates pattern
4. ✅ Thread-safety hardening: Documented requirements
5. ✅ Cleanup: Dev assertions added, no workarounds present

## Files Changed

- `java/src/main/java/com/astraeus/native_api/NativeEngine.java` (core fix)
- `java/src/main/java/com/astraeus/rendering/FxViewport.java` (assertions)
- `java/src/main/java/com/astraeus/rendering/FxViewportV2.java` (assertions)
- `docs/BYTEBUFFER_STATE_FIX.md` (documentation)
- `docs/BYTEBUFFER_FIX_TEST_SCENARIOS.md` (test scenarios)
- `docs/BYTEBUFFER_FIX_SUMMARY.md` (this file)

## Commits

1. Initial fix: Stable buffers and getViewportBuffer() method
2. Documentation: Comprehensive guides and test scenarios
3. Code review fixes: Null checks, validation, thread-safety docs
4. Final fix: Proper overflow handling

## Status

**COMPLETE** - Ready for testing

All deliverables met, all code review feedback addressed, comprehensive documentation provided.

---

**Principle**: JavaFX PixelBuffer owns the ByteBuffer's state; engine code must not touch position/limit/mark after handing it over.
