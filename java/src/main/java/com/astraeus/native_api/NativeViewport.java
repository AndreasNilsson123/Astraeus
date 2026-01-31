package com.astraeus.native_api;

import com.astraeus.native_api.model.PixelBufferView;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.MemoryLayout;
import java.lang.invoke.VarHandle;
import java.nio.ByteBuffer;

/**
 * Java wrapper for native viewport handle.
 * Manages rendering viewport, pixel buffers, and active camera.
 * 
 * <p>This class provides multi-viewport support by wrapping individual
 * native viewport handles. Each viewport has its own backing buffers
 * for color and ID data.</p>
 * 
 * <p><b>Thread Safety:</b> This class is NOT thread-safe. All methods must
 * be called from the JavaFX Application Thread.</p>
 */
public class NativeViewport implements AutoCloseable {
    
    private MemorySegment viewportHandle;
    private final NativeEngine engine;
    private final Arena arena;
    private boolean closed = false;
    
    // Cached backing buffers for stable pointer access
    private MemorySegment colorDataSeg;
    private ByteBuffer colorByteBufferStable;
    private long colorAddr = 0;
    private int colorBackingSize = 0;
    
    private MemorySegment idDataSeg;
    private ByteBuffer idByteBufferStable;
    private long idAddr = 0;
    private int idBackingSize = 0;
    
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
     * 
     * <p>The returned PixelBufferView has a stable ByteBuffer attached that can be
     * safely used with JavaFX PixelBuffer. The backing memory is allocated once
     * and never reallocated, ensuring pointer stability.</p>
     * 
     * @return PixelBufferView with stable ByteBuffer attached
     * @throws RuntimeException if buffer retrieval fails
     */
    public PixelBufferView getColorBuffer() {
        checkClosed();
        
        // Allocate struct for native call
        MemorySegment viewStruct = arena.allocate(EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT);
        
        try {
            int result = (int) EngineBindings.VIEWPORT_GET_COLOR.invoke(viewportHandle, viewStruct);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException("Failed to get color buffer, error code: " + result);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get color buffer", e);
        }
        
        // Create PixelBufferView from struct
        PixelBufferView view = new PixelBufferView(viewStruct);
        
        long addr = view.dataAddress();
        int backingSize = view.getMaxBackingSize();
        
        // Calculate needed size
        int needed;
        try {
            needed = Math.multiplyExact(view.getStride(), view.getHeight());
        } catch (ArithmeticException e) {
            throw new IllegalStateException("Viewport size calculation overflowed: stride=" + 
                    view.getStride() + " * height=" + view.getHeight(), e);
        }
        
        if (addr == 0 || backingSize <= 0) {
            throw new IllegalStateException("Invalid color view: addr=" + addr + " backingSize=" + backingSize);
        }
        if (needed > backingSize) {
            throw new IllegalStateException("Color view size mismatch: needed=" + needed + 
                    " backingSize=" + backingSize);
        }
        
        // Create/reuse stable ByteBuffer (only rebuild if address/size changed)
        if (colorByteBufferStable == null || colorAddr != addr || colorBackingSize != backingSize) {
            colorAddr = addr;
            colorBackingSize = backingSize;
            
            // Reinterpret with viewport lifetime arena
            colorDataSeg = MemorySegment.ofAddress(addr).reinterpret(backingSize, arena, null);
            
            // Create stable ByteBuffer (position=0, limit=capacity, NEVER mutate after this)
            colorByteBufferStable = colorDataSeg.asByteBuffer();
        }
        
        // Attach stable ByteBuffer to view
        view.attachByteBuffer(colorByteBufferStable, needed);
        
        return view;
    }
    
    /**
     * Get the ID buffer (for picking).
     * 
     * <p>The returned PixelBufferView has a stable ByteBuffer attached that can be
     * used for entity picking. The backing memory is allocated once and never
     * reallocated, ensuring pointer stability.</p>
     * 
     * @return PixelBufferView with stable ByteBuffer attached
     * @throws RuntimeException if buffer retrieval fails
     */
    public PixelBufferView getIdBuffer() {
        checkClosed();
        
        // Allocate struct for native call
        MemorySegment viewStruct = arena.allocate(EngineBindings.PIXEL_BUFFER_VIEW_LAYOUT);
        
        try {
            int result = (int) EngineBindings.VIEWPORT_GET_IDBUFFER.invoke(viewportHandle, viewStruct);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException("Failed to get ID buffer, error code: " + result);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get ID buffer", e);
        }
        
        // Create PixelBufferView from struct
        PixelBufferView view = new PixelBufferView(viewStruct);
        
        long addr = view.dataAddress();
        int backingSize = view.getMaxBackingSize();
        
        // Calculate needed size (4 bytes per pixel for R32UI)
        int needed;
        try {
            needed = Math.multiplyExact(view.getStride(), view.getHeight());
        } catch (ArithmeticException e) {
            throw new IllegalStateException("Viewport size calculation overflowed: stride=" + 
                    view.getStride() + " * height=" + view.getHeight(), e);
        }
        
        if (addr == 0 || backingSize <= 0) {
            throw new IllegalStateException("Invalid ID view: addr=" + addr + " backingSize=" + backingSize);
        }
        if (needed > backingSize) {
            throw new IllegalStateException("ID view size mismatch: needed=" + needed + 
                    " backingSize=" + backingSize);
        }
        
        // Create/reuse stable ByteBuffer (only rebuild if address/size changed)
        if (idByteBufferStable == null || idAddr != addr || idBackingSize != backingSize) {
            idAddr = addr;
            idBackingSize = backingSize;
            
            // Reinterpret with viewport lifetime arena
            idDataSeg = MemorySegment.ofAddress(addr).reinterpret(backingSize, arena, null);
            
            // Create stable ByteBuffer (position=0, limit=capacity, NEVER mutate after this)
            idByteBufferStable = idDataSeg.asByteBuffer();
        }
        
        // Attach stable ByteBuffer to view
        view.attachByteBuffer(idByteBufferStable, needed);
        
        return view;
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
}
