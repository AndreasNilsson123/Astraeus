package com.astraeus.native_api;

import java.lang.foreign.*;
import java.lang.invoke.VarHandle;

/**
 * High-level Java wrapper around the native Astraeus engine.
 * Manages native memory and provides a safe API.
 */
public class NativeEngine implements AutoCloseable {
    
    private MemorySegment engineHandle;
    private final Arena arena;
    private boolean closed = false;
    
    /**
     * Create a new engine instance.
     * @param width Initial viewport width
     * @param height Initial viewport height
     * @param enableValidation Enable validation layers
     */
    public NativeEngine(int width, int height, boolean enableValidation) {
        this.arena = Arena.ofShared();
        
        // Allocate and populate EngineConfig using layout accessors
        MemorySegment config = arena.allocate(EngineBindings.ENGINE_CONFIG_LAYOUT);
        
        // Use field offsets from layout for safe, platform-independent access
        VarHandle widthHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("initial_width"));
        VarHandle heightHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("initial_height"));
        VarHandle validationHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("enable_validation"));
        VarHandle debugHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("enable_debug_output"));
        VarHandle logPathHandle = EngineBindings.ENGINE_CONFIG_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("log_file_path"));
        
        widthHandle.set(config, 0L, width);
        heightHandle.set(config, 0L, height);
        validationHandle.set(config, 0L, enableValidation);
        debugHandle.set(config, 0L, false);
        logPathHandle.set(config, 0L, MemorySegment.NULL);
        
        try {
            // Call native function
            engineHandle = (MemorySegment) EngineBindings.CREATE_ENGINE.invoke(config);
            
            if (engineHandle == null || engineHandle.equals(MemorySegment.NULL)) {
                throw new RuntimeException("Failed to create native engine");
            }
            
            // Verify engine is valid
            boolean valid = (boolean) EngineBindings.IS_VALID.invoke(engineHandle);
            if (!valid) {
                throw new RuntimeException("Native engine is not valid after creation");
            }
            
            System.out.println("[NativeEngine] Engine created successfully");
            
        } catch (Throwable e) {
            arena.close();
            throw new RuntimeException("Failed to initialize native engine", e);
        }
    }
    
    /**
     * Begin a new frame.
     * @param deltaTime Time since last frame in seconds
     */
    public void beginFrame(double deltaTime) {
        checkClosed();
        try {
            EngineBindings.BEGIN_FRAME.invoke(engineHandle, deltaTime);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to begin frame", e);
        }
    }
    
    /**
     * End the current frame.
     */
    public void endFrame() {
        checkClosed();
        try {
            EngineBindings.END_FRAME.invoke(engineHandle);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to end frame", e);
        }
    }
    
    /**
     * Resize the viewport.
     * @param width New width
     * @param height New height
     */
    public void resizeViewport(int width, int height) {
        checkClosed();
        try {
            EngineBindings.RESIZE_VIEWPORT.invoke(engineHandle, width, height);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to resize viewport", e);
        }
    }
    
    /**
     * Create a new entity.
     * @return Entity ID
     */
    public int createEntity() {
        checkClosed();
        try {
            return (int) EngineBindings.CREATE_ENTITY.invoke(engineHandle);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to create entity", e);
        }
    }
    
    /**
     * Destroy an entity.
     * @param entityId Entity ID
     */
    public void destroyEntity(int entityId) {
        checkClosed();
        try {
            EngineBindings.DESTROY_ENTITY.invoke(engineHandle, entityId);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to destroy entity", e);
        }
    }
    
    /**
     * Check if engine is valid.
     * @return true if valid
     */
    public boolean isValid() {
        if (closed) {
            return false;
        }
        try {
            return (boolean) EngineBindings.IS_VALID.invoke(engineHandle);
        } catch (Throwable e) {
            return false;
        }
    }
    
    private void checkClosed() {
        if (closed) {
            throw new IllegalStateException("Engine has been closed");
        }
    }
    
    @Override
    public void close() {
        if (closed) {
            return;
        }
        
        try {
            if (engineHandle != null && !engineHandle.equals(MemorySegment.NULL)) {
                EngineBindings.DESTROY_ENGINE.invoke(engineHandle);
            }
        } catch (Throwable e) {
            System.err.println("Error destroying engine: " + e.getMessage());
        } finally {
            arena.close();
            closed = true;
            System.out.println("[NativeEngine] Engine closed");
        }
    }
}
