package com.astraeus.native_api;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.invoke.VarHandle;

/**
 * Java wrapper for native viewport handle.
 * Manages rendering viewport, pixel buffers, and active camera.
 */
public class NativeViewport implements AutoCloseable {
    
    private MemorySegment viewportHandle;
    private final NativeEngine engine;
    private final Arena arena;
    private boolean closed = false;
    
    /**
     * Package-private constructor (created by NativeEngine).
     */
    NativeViewport(MemorySegment viewportHandle, NativeEngine engine, Arena arena) {
        this.viewportHandle = viewportHandle;
        this.engine = engine;
        this.arena = arena;
    }
    
    /**
     * Resize the viewport.
     */
    public void resize(int width, int height) {
        checkClosed();
        try {
            int result = (int) EngineBindings.VIEWPORT_RESIZE.invoke(viewportHandle, width, height);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException("Failed to resize viewport, error code: " + result);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to resize viewport", e);
        }
    }
    
    /**
     * Get the color buffer (for rendering).
     */
    public PixelBufferView getColorBuffer() {
        checkClosed();
        try {
            MemorySegment bufferView = arena.allocate(EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT);
            int result = (int) EngineBindings.VIEWPORT_GET_COLOR.invoke(viewportHandle, bufferView);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException("Failed to get color buffer, error code: " + result);
            }
            
            return extractPixelBufferView(bufferView);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get color buffer", e);
        }
    }
    
    /**
     * Get the ID buffer (for picking).
     */
    public PixelBufferView getIdBuffer() {
        checkClosed();
        try {
            MemorySegment bufferView = arena.allocate(EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT);
            int result = (int) EngineBindings.VIEWPORT_GET_IDBUFFER.invoke(viewportHandle, bufferView);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException("Failed to get ID buffer, error code: " + result);
            }
            
            return extractPixelBufferView(bufferView);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get ID buffer", e);
        }
    }
    
    /**
     * Get the active camera for this viewport.
     */
    public NativeCamera getActiveCamera() {
        checkClosed();
        try {
            MemorySegment cameraHandlePtr = arena.allocate(java.lang.foreign.ValueLayout.ADDRESS);
            int result = (int) EngineBindings.CAMERA_GET_ACTIVE.invoke(viewportHandle, cameraHandlePtr);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException("Failed to get active camera, error code: " + result);
            }
            
            MemorySegment cameraHandle = cameraHandlePtr.get(
                java.lang.foreign.ValueLayout.ADDRESS, 0L);
            
            return new NativeCamera(cameraHandle, arena);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get active camera", e);
        }
    }
    
    @Override
    public void close() {
        if (closed) {
            return;
        }
        
        try {
            EngineBindings.VIEWPORT_DESTROY.invoke(viewportHandle);
            viewportHandle = null;
            closed = true;
        } catch (Throwable e) {
            throw new RuntimeException("Failed to destroy viewport", e);
        }
    }
    
    private void checkClosed() {
        if (closed) {
            throw new IllegalStateException("Viewport has been closed");
        }
    }
    
    /**
     * Extract PixelBufferView from native struct.
     */
    private PixelBufferView extractPixelBufferView(MemorySegment bufferView) {
        VarHandle dataHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
            java.lang.foreign.MemoryLayout.PathElement.groupElement("data"));
        VarHandle widthHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
            java.lang.foreign.MemoryLayout.PathElement.groupElement("width"));
        VarHandle heightHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
            java.lang.foreign.MemoryLayout.PathElement.groupElement("height"));
        VarHandle strideHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
            java.lang.foreign.MemoryLayout.PathElement.groupElement("stride"));
        VarHandle formatHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
            java.lang.foreign.MemoryLayout.PathElement.groupElement("format"));
        VarHandle maxWidthHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
            java.lang.foreign.MemoryLayout.PathElement.groupElement("max_backing_width"));
        VarHandle maxHeightHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
            java.lang.foreign.MemoryLayout.PathElement.groupElement("max_backing_height"));
        VarHandle maxSizeHandle = EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT.varHandle(
            java.lang.foreign.MemoryLayout.PathElement.groupElement("max_backing_size"));
        
        MemorySegment data = (MemorySegment) dataHandle.get(bufferView, 0L);
        int width = (int) widthHandle.get(bufferView, 0L);
        int height = (int) heightHandle.get(bufferView, 0L);
        int stride = (int) strideHandle.get(bufferView, 0L);
        int format = (int) formatHandle.get(bufferView, 0L);
        int maxWidth = (int) maxWidthHandle.get(bufferView, 0L);
        int maxHeight = (int) maxHeightHandle.get(bufferView, 0L);
        int maxSize = (int) maxSizeHandle.get(bufferView, 0L);
        
        return new PixelBufferView(data, width, height, stride, format, 
                                   maxWidth, maxHeight, maxSize);
    }
    
    /**
     * Pixel buffer view (zero-copy access to native memory).
     */
    public record PixelBufferView(
        MemorySegment data,
        int width, int height, int stride, int format,
        int maxBackingWidth, int maxBackingHeight, int maxBackingSize
    ) {
        /**
         * Get a reinterpreted view of the buffer data.
         */
        public MemorySegment asReinterpretedBuffer() {
            if (data == null || data.equals(MemorySegment.NULL)) {
                throw new IllegalStateException("Buffer data is null");
            }
            // Reinterpret with the actual backing size
            return data.reinterpret(maxBackingSize);
        }
        
        /**
         * Check if buffer is valid.
         */
        public boolean isValid() {
            return data != null && !data.equals(MemorySegment.NULL) && width > 0 && height > 0;
        }
    }
}
