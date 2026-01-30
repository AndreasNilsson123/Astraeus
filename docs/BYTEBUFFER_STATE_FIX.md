# ByteBuffer State Corruption Fix

## Problem

JavaFX's `PixelBuffer` requires that the `ByteBuffer` object passed to it maintains stable state (position/limit/mark) throughout its lifetime. The previous implementation was mutating the buffer's `position()` and `limit()` on every call to `getColorBuffer()` and `getIdBuffer()`, which violated this contract and caused exceptions like:

```
BaseTexture.checkUpdateParams: Upload requires ... elements
```

## Root Cause

In `NativeEngine.java`, the following code was executed on **every frame**:

```java
// OLD CODE (BROKEN)
colorByteBuffer.position(0);
colorByteBuffer.limit(needed);  // where needed = stride * height
view.attachByteBuffer(colorByteBuffer);
```

This mutated the ByteBuffer state after it had been given to JavaFX's PixelBuffer, breaking JavaFX's internal assumptions about buffer state.

## Solution

The fix implements the principle: **JavaFX PixelBuffer owns the ByteBuffer's state; engine code must not touch position/limit/mark after handing it over.**

### Key Changes

1. **Stable Buffer Creation** (`NativeEngine.java`)
   - Create `colorByteBufferStable` with `position=0, limit=capacity` **once**
   - Never mutate its state again
   - This buffer is given to JavaFX PixelBuffer and remains untouched

2. **Viewport-Sized Access** (`PixelBufferView`)
   - Store `viewportByteSize` for internal use
   - Add `getViewportBuffer()` method that returns **duplicates** with sized limits
   - Duplicates share native memory but have independent position/limit/mark state

3. **API Documentation**
   - `getByteBuffer()`: Returns stable buffer for JavaFX (DO NOT mutate)
   - `getViewportBuffer()`: Returns duplicate for internal sized access (safe to mutate)

### Code Structure

```java
// NativeEngine.java - Buffer creation (once per engine lifecycle)
if (colorByteBufferStable == null || colorAddr != addr || colorBackingSize != backingSize) {
    colorDataSeg = MemorySegment.ofAddress(addr).reinterpret(backingSize, arena, null);
    colorByteBufferStable = colorDataSeg.asByteBuffer();
    colorByteBufferStable.position(0);
    colorByteBufferStable.limit(colorByteBufferStable.capacity());  // Set once, never changed
}

// Attach stable buffer and viewport size
view.attachByteBuffer(colorByteBufferStable, needed);
```

```java
// PixelBufferView - Safe access patterns
public class PixelBufferView {
    private ByteBuffer stableByteBuffer;  // For JavaFX - immutable state
    private int viewportByteSize;          // Current viewport size in bytes
    
    /** Get stable buffer for JavaFX PixelBuffer (DO NOT mutate position/limit) */
    public ByteBuffer getByteBuffer() {
        return stableByteBuffer;
    }
    
    /** Get duplicate with sized limit for internal operations (safe to mutate) */
    public ByteBuffer getViewportBuffer() {
        ByteBuffer duplicate = stableByteBuffer.duplicate();
        duplicate.position(0);
        duplicate.limit(viewportByteSize);
        return duplicate;
    }
}
```

### Usage Examples

#### ✅ CORRECT: JavaFX Integration
```java
// One-time setup
NativeEngine.PixelBufferView colorView = engine.getColorBuffer();
ByteBuffer backingBuffer = colorView.getByteBuffer();  // Stable buffer
pixelBuffer = new PixelBuffer<>(maxW, maxH, backingBuffer, format);

// Per-frame: Never touch the buffer state
pixelBuffer.updateBuffer(pb -> {
    return new Rectangle2D(0, 0, currentWidth, currentHeight);
});
```

#### ✅ CORRECT: Internal Sized Access
```java
// If you need to read viewport-sized data internally
NativeEngine.PixelBufferView colorView = engine.getColorBuffer();
ByteBuffer viewportData = colorView.getViewportBuffer();  // Safe duplicate
// Now you can mutate viewportData's position/limit as needed
```

#### ❌ INCORRECT: Mutating JavaFX's Buffer
```java
// DON'T DO THIS!
ByteBuffer buffer = colorView.getByteBuffer();
buffer.position(0);        // WRONG: Mutates JavaFX's buffer
buffer.limit(newSize);     // WRONG: Mutates JavaFX's buffer
```

## Debugging

Enable dev-mode assertions to detect state corruption:

```bash
java -Dastraeus.debug.assertBufferState=true ...
```

This will log warnings if the buffer state drifts from expected values (position=0, limit=capacity).

## Testing

To verify the fix:

1. **Extended Rendering Test**: Run the application with continuous rendering for 1000+ frames
2. **Viewport Resize Test**: Dynamically resize the viewport multiple times during rendering
3. **Debug Assertions**: Enable `-Dastraeus.debug.assertBufferState=true` and verify no warnings

Expected results:
- No `BaseTexture.checkUpdateParams` exceptions
- No "Upload requires ... elements" errors
- Debug assertions show stable buffer state across all frames and resizes

## Related Files

- `java/src/main/java/com/astraeus/native_api/NativeEngine.java` - Core fix (getColorBuffer/getIdBuffer)
- `java/src/main/java/com/astraeus/rendering/FxViewport.java` - JavaFX integration with assertions
- `java/src/main/java/com/astraeus/rendering/FxViewportV2.java` - Enhanced viewport with assertions

## Acceptance Criteria

✅ ByteBuffer handed to PixelBuffer has position == 0  
✅ ByteBuffer handed to PixelBuffer has limit == capacity  
✅ Buffer state stays stable across frames and resizes  
✅ No per-frame buffer-state manipulation required  
✅ Dev-mode diagnostics can verify invariants  
✅ No exceptions during extended rendering or resizing
