package com.astraeus.native_api;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.invoke.VarHandle;

/**
 * Java wrapper for native camera handle.
 * Provides safe access to camera descriptor (position, target, projection).
 * Note: Camera handles are lightweight and do NOT own the actual camera.
 */
public class NativeCamera implements AutoCloseable {
    
    private MemorySegment cameraHandle;
    private final Arena arena;
    private boolean closed = false;
    
    /**
     * Package-private constructor (created by NativeViewport).
     */
    NativeCamera(MemorySegment cameraHandle, Arena arena) {
        this.cameraHandle = cameraHandle;
        this.arena = arena;
    }
    
    /**
     * Get the current camera descriptor.
     */
    public CameraDesc getDesc() {
        checkClosed();
        try {
            MemorySegment descSegment = arena.allocate(EngineBindings.CAMERA_DESC_LAYOUT);
            int result = (int) EngineBindings.CAMERA_GET_DESC.invoke(cameraHandle, descSegment);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException("Failed to get camera descriptor, error code: " + result);
            }
            
            // Extract fields from struct
            VarHandle pos_x_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("pos_x"));
            VarHandle pos_y_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("pos_y"));
            VarHandle pos_z_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("pos_z"));
            
            VarHandle target_x_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("target_x"));
            VarHandle target_y_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("target_y"));
            VarHandle target_z_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("target_z"));
            
            VarHandle up_x_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("up_x"));
            VarHandle up_y_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("up_y"));
            VarHandle up_z_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("up_z"));
            
            VarHandle fov_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("fov_degrees"));
            VarHandle near_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("near_plane"));
            VarHandle far_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("far_plane"));
            VarHandle mode_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("mode"));
            
            return new CameraDesc(
                (float) pos_x_handle.get(descSegment, 0L),
                (float) pos_y_handle.get(descSegment, 0L),
                (float) pos_z_handle.get(descSegment, 0L),
                (float) target_x_handle.get(descSegment, 0L),
                (float) target_y_handle.get(descSegment, 0L),
                (float) target_z_handle.get(descSegment, 0L),
                (float) up_x_handle.get(descSegment, 0L),
                (float) up_y_handle.get(descSegment, 0L),
                (float) up_z_handle.get(descSegment, 0L),
                (float) fov_handle.get(descSegment, 0L),
                (float) near_handle.get(descSegment, 0L),
                (float) far_handle.get(descSegment, 0L),
                (int) mode_handle.get(descSegment, 0L)
            );
            
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get camera descriptor", e);
        }
    }
    
    /**
     * Set the camera descriptor.
     */
    public void setDesc(CameraDesc desc) {
        checkClosed();
        try {
            MemorySegment descSegment = arena.allocate(EngineBindings.CAMERA_DESC_LAYOUT);
            
            // Populate struct fields
            VarHandle pos_x_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("pos_x"));
            VarHandle pos_y_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("pos_y"));
            VarHandle pos_z_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("pos_z"));
            
            VarHandle target_x_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("target_x"));
            VarHandle target_y_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("target_y"));
            VarHandle target_z_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("target_z"));
            
            VarHandle up_x_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("up_x"));
            VarHandle up_y_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("up_y"));
            VarHandle up_z_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("up_z"));
            
            VarHandle fov_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("fov_degrees"));
            VarHandle near_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("near_plane"));
            VarHandle far_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("far_plane"));
            VarHandle mode_handle = EngineBindings.CAMERA_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("mode"));
            
            pos_x_handle.set(descSegment, 0L, desc.posX());
            pos_y_handle.set(descSegment, 0L, desc.posY());
            pos_z_handle.set(descSegment, 0L, desc.posZ());
            target_x_handle.set(descSegment, 0L, desc.targetX());
            target_y_handle.set(descSegment, 0L, desc.targetY());
            target_z_handle.set(descSegment, 0L, desc.targetZ());
            up_x_handle.set(descSegment, 0L, desc.upX());
            up_y_handle.set(descSegment, 0L, desc.upY());
            up_z_handle.set(descSegment, 0L, desc.upZ());
            fov_handle.set(descSegment, 0L, desc.fovDegrees());
            near_handle.set(descSegment, 0L, desc.nearPlane());
            far_handle.set(descSegment, 0L, desc.farPlane());
            mode_handle.set(descSegment, 0L, desc.mode());
            
            int result = (int) EngineBindings.CAMERA_SET_DESC.invoke(cameraHandle, descSegment);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException("Failed to set camera descriptor, error code: " + result);
            }
            
        } catch (Throwable e) {
            throw new RuntimeException("Failed to set camera descriptor", e);
        }
    }
    
    @Override
    public void close() {
        if (closed) {
            return;
        }
        
        try {
            // Camera handles are lightweight - no native cleanup needed in MVP
            // In future, if cameras become heavier resources, add cleanup here
            cameraHandle = null;
            closed = true;
        } catch (Throwable e) {
            throw new RuntimeException("Failed to close camera handle", e);
        }
    }
    
    private void checkClosed() {
        if (closed) {
            throw new IllegalStateException("Camera handle has been closed");
        }
    }
    
    /**
     * Camera descriptor record (immutable).
     */
    public record CameraDesc(
        float posX, float posY, float posZ,
        float targetX, float targetY, float targetZ,
        float upX, float upY, float upZ,
        float fovDegrees, float nearPlane, float farPlane,
        int mode
    ) {
        /**
         * Create with default values.
         */
        public static CameraDesc defaults() {
            return new CameraDesc(
                10.0f, 10.0f, 10.0f,  // position
                0.0f, 0.0f, 0.0f,     // target
                0.0f, 1.0f, 0.0f,     // up
                60.0f,                // fov
                0.1f, 1000.0f,        // near, far
                EngineBindings.CAMERA_MODE_ORBIT
            );
        }
        
        /**
         * Create a copy with modified position.
         */
        public CameraDesc withPosition(float x, float y, float z) {
            return new CameraDesc(x, y, z, targetX, targetY, targetZ, upX, upY, upZ,
                                  fovDegrees, nearPlane, farPlane, mode);
        }
        
        /**
         * Create a copy with modified target.
         */
        public CameraDesc withTarget(float x, float y, float z) {
            return new CameraDesc(posX, posY, posZ, x, y, z, upX, upY, upZ,
                                  fovDegrees, nearPlane, farPlane, mode);
        }
        
        /**
         * Create a copy with modified FOV.
         */
        public CameraDesc withFov(float fov) {
            return new CameraDesc(posX, posY, posZ, targetX, targetY, targetZ, upX, upY, upZ,
                                  fov, nearPlane, farPlane, mode);
        }
    }
}
