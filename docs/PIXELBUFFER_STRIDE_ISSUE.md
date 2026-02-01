# PixelBuffer Stride Issue Analysis

## Problem Statement
Split-view / top-half pixel corruption observed in JavaFX viewport when rendering at dimensions smaller than the backing buffer size.

## Root Cause
JavaFX `PixelBuffer` does NOT support row stride. It expects tightly-packed pixel data where:
```
row_offset = y * width * bytes_per_pixel
```

However, our native engine uses a stride-based layout where:
```
row_offset = y * stride
stride = max_backing_width * bytes_per_pixel
```

When `current_width < max_backing_width`, the data layout doesn't match JavaFX's expectations.

## Example Scenario
- Backing buffer: 2560 x 1440
- Current viewport: 1280 x 720
- Stride: 2560 * 4 = 10240 bytes/row
- Expected by JavaFX: 1280 * 4 = 5120 bytes/row

### What Happens:
1. Native engine fills pixel data with stride=10240
2. JavaFX PixelBuffer created with dimensions 2560x1440 (correct)
3. ImageView.setViewport() crops to 1280x720 (correct)
4. PixelBuffer.updateBuffer() called with dirty region (0,0,1280,720)
5. JavaFX reads data assuming no stride, causing it to read from wrong memory locations

## Solutions

### Option 1: Copy to Tightly-Packed Buffer (RECOMMENDED)
Create a Java-side buffer that is tightly packed for the current viewport dimensions:
- Pros: Clean, no native changes needed
- Cons: One extra copy per frame

### Option 2: Always Render at Full Backing Size
Never resize the native viewport, always render at max backing size:
- Pros: No stride issues, simple
- Cons: Wastes GPU resources, fills pixels that may not be visible

### Option 3: Use WritablePixelFormat with Custom Setter
Implement a custom pixel setter that handles stride:
- Pros: No data copy
- Cons: Significantly more complex, may have performance issues

## Recommended Implementation: Option 1

Create a "viewport-sized" tightly-packed buffer in Java and copy from the native strided buffer:

```java
// In FxViewport or PixelBufferManager:
private ByteBuffer tightlyPackedBuffer;
private PixelBuffer<ByteBuffer> pixelBuffer;

public void updateDisplay() {
    // Get native buffer with stride
    PixelBufferView nativeView = engine.getColorBuffer();
    ByteBuffer nativeBuffer = nativeView.getByteBuffer();
    int stride = nativeView.getStride();
    int width = currentWidth;
    int height = currentHeight;
    int bytesPerPixel = 4;
    
    // Allocate/resize tightly-packed buffer if needed
    int requiredSize = width * height * bytesPerPixel;
    if (tightlyPackedBuffer == null || tightlyPackedBuffer.capacity() < requiredSize) {
        tightlyPackedBuffer = ByteBuffer.allocateDirect(requiredSize);
        pixelBuffer = new PixelBuffer<>(width, height, tightlyPackedBuffer, format);
        writableImage = new WritableImage(pixelBuffer);
    }
    
    // Copy with stride handling
    tightlyPackedBuffer.clear();
    for (int y = 0; y < height; y++) {
        int srcOffset = y * stride;
        int dstOffset = y * width * bytesPerPixel;
        nativeBuffer.position(srcOffset);
        nativeBuffer.limit(srcOffset + width * bytesPerPixel);
        tightlyPackedBuffer.position(dstOffset);
        tightlyPackedBuffer.put(nativeBuffer);
    }
    
    tightlyPackedBuffer.clear();
    pixelBuffer.updateBuffer(pb -> new Rectangle2D(0, 0, width, height));
}
```

## Alternative: Fix Native Stride
Modify native RenderDevice to use current viewport dimensions for stride:
```cpp
out_view.stride = color_backing_.current_width * 4;  // Viewport stride, not backing
```

But this would require repacking the data on the native side or changing the render target layout, which is more invasive.

## Conclusion
Option 1 (copy to tightly-packed buffer in Java) is the safest and cleanest solution that maintains the existing native API contract while fixing the JavaFX integration issue.
