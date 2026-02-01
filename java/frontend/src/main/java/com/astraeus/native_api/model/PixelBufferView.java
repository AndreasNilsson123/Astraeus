package com.astraeus.native_api.model;

import com.astraeus.native_api.EngineBindings;

import java.lang.foreign.MemoryLayout;
import java.lang.foreign.MemorySegment;
import java.lang.invoke.VarHandle;
import java.nio.ByteBuffer;

/**
 * Wrapper for PixelBufferView struct from the native engine.
 * Provides safe access to pixel buffer data without memory hazards.
 * 
 * <p>This class manages stable pixel buffer memory that can be safely used
 * with JavaFX PixelBuffer. The backing memory is allocated once and never
 * reallocated, ensuring pointer stability.</p>
 * 
 * <p><b>Thread Safety:</b> This class is NOT thread-safe. All methods must be
 * called from the same thread (typically the JavaFX Application Thread).</p>
 * 
 * @see com.astraeus.native_api.NativeEngine#getColorBuffer()
 * @see com.astraeus.native_api.NativeEngine#getIdBuffer()
 */
public class PixelBufferView {
    private final long dataAddress;
    private final int width;
    private final int height;
    private final int stride;
    private final int format;
    private final int maxBackingWidth;
    private final int maxBackingHeight;
    private final int maxBackingSize;

    // STABLE ByteBuffer for JavaFX (position=0, limit=capacity, never mutated)
    private ByteBuffer stableByteBuffer;
    
    // Current viewport size in bytes (for creating duplicates)
    private int viewportByteSize;

    /**
     * Create a PixelBufferView from a native PixelBufferView struct.
     * 
     * <p>This constructor reads the struct fields from native memory and
     * creates an immutable Java view of the data.</p>
     * 
     * @param structSegment Memory segment containing the native PixelBufferView struct
     * @throws NullPointerException if structSegment is null
     */
    public PixelBufferView(MemorySegment structSegment) {
        VarHandle dataHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("data"));
        VarHandle widthHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("width"));
        VarHandle heightHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("height"));
        VarHandle strideHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("stride"));
        VarHandle formatHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("format"));
        VarHandle maxWidthHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("max_backing_width"));
        VarHandle maxHeightHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("max_backing_height"));
        VarHandle maxSizeHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
                MemoryLayout.PathElement.groupElement("max_backing_size"));

        MemorySegment dataPtr = (MemorySegment) dataHandle.get(structSegment, 0L);
        this.dataAddress = (dataPtr == null || dataPtr.equals(MemorySegment.NULL)) ? 0L : dataPtr.address();

        this.width = (int) widthHandle.get(structSegment, 0L);
        this.height = (int) heightHandle.get(structSegment, 0L);
        this.stride = (int) strideHandle.get(structSegment, 0L);
        this.format = (int) formatHandle.get(structSegment, 0L);
        this.maxBackingWidth = (int) maxWidthHandle.get(structSegment, 0L);
        this.maxBackingHeight = (int) maxHeightHandle.get(structSegment, 0L);
        this.maxBackingSize = (int) maxSizeHandle.get(structSegment, 0L);
    }

    /**
     * Get the native memory address of the pixel data.
     * 
     * @return Memory address (0 if invalid)
     */
    public long dataAddress() { 
        return dataAddress; 
    }

    /**
     * Get the current viewport width in pixels.
     * 
     * @return Viewport width
     */
    public int getWidth() { 
        return width; 
    }

    /**
     * Get the current viewport height in pixels.
     * 
     * @return Viewport height
     */
    public int getHeight() { 
        return height; 
    }

    /**
     * Get the row stride in bytes.
     * 
     * <p>The stride is the number of bytes per row, which may be larger than
     * width * bytesPerPixel due to alignment requirements.</p>
     * 
     * @return Row stride in bytes
     */
    public int getStride() { 
        return stride; 
    }

    /**
     * Get the pixel format.
     * 
     * @return Pixel format constant (see {@link EngineBindings})
     */
    public int getFormat() { 
        return format; 
    }

    /**
     * Get the maximum backing buffer width.
     * 
     * @return Maximum width in pixels
     */
    public int getMaxBackingWidth() { 
        return maxBackingWidth; 
    }

    /**
     * Get the maximum backing buffer height.
     * 
     * @return Maximum height in pixels
     */
    public int getMaxBackingHeight() { 
        return maxBackingHeight; 
    }

    /**
     * Get the maximum backing buffer size in bytes.
     * 
     * @return Maximum size in bytes
     */
    public int getMaxBackingSize() { 
        return maxBackingSize; 
    }

    /**
     * Get the stable ByteBuffer for JavaFX PixelBuffer.
     * 
     * <p><b>WARNING:</b> This buffer has position=0 and limit=capacity.
     * DO NOT mutate its position/limit/mark after giving it to JavaFX PixelBuffer.
     * The buffer state must remain stable for JavaFX's lifetime.</p>
     * 
     * <p><b>Thread Safety:</b> Must be called from JavaFX Application Thread only.</p>
     * 
     * @return Stable ByteBuffer with immutable state
     */
    public ByteBuffer getByteBuffer() { 
        return stableByteBuffer; 
    }
    
    /**
     * Get a duplicate ByteBuffer sized to the current viewport.
     * 
     * <p>This creates a new ByteBuffer object that shares the same native memory
     * but has its own position/limit/mark state. Use this when you need a sized
     * view for internal operations (e.g., reading specific viewport data).</p>
     * 
     * <p><b>IMPORTANT:</b> This method returns valid data only after calling
     * {@link com.astraeus.native_api.NativeEngine#getColorBuffer()} or
     * {@link com.astraeus.native_api.NativeEngine#getIdBuffer()}.
     * The viewport size is captured at that moment. If the viewport is resized,
     * you must call those methods again before calling this method.</p>
     * 
     * @return A duplicate ByteBuffer with limit set to viewportByteSize
     * @throws IllegalStateException if the buffer is not initialized or viewportByteSize is invalid
     */
    public ByteBuffer getViewportBuffer() {
        if (stableByteBuffer == null) {
            throw new IllegalStateException("Buffer not initialized - call getColorBuffer() or getIdBuffer() first");
        }
        if (viewportByteSize <= 0) {
            throw new IllegalStateException("Invalid viewport size: " + viewportByteSize + 
                    " - call getColorBuffer() or getIdBuffer() first");
        }
        if (viewportByteSize > stableByteBuffer.capacity()) {
            throw new IllegalStateException("Viewport size " + viewportByteSize + 
                    " exceeds buffer capacity " + stableByteBuffer.capacity());
        }
        
        // Create a duplicate with sized limit for internal use
        ByteBuffer duplicate = stableByteBuffer.duplicate();
        duplicate.position(0);
        duplicate.limit(viewportByteSize);
        return duplicate;
    }

    /**
     * Attach a ByteBuffer to this view (package-private for NativeEngine).
     * 
     * @param bb The stable ByteBuffer
     * @param viewportByteSize The current viewport size in bytes
     */
    public void attachByteBuffer(ByteBuffer bb, int viewportByteSize) {
        this.stableByteBuffer = bb;
        this.viewportByteSize = viewportByteSize;
    }
}
