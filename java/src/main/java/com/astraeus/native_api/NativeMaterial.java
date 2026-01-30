package com.astraeus.native_api;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.invoke.VarHandle;

/**
 * Java wrapper for native material handle.
 * Provides safe access to material parameters (PBR-style).
 */
public class NativeMaterial implements AutoCloseable {
    
    private MemorySegment materialHandle;
    private final Arena arena;
    private boolean closed = false;
    
    /**
     * Package-private constructor (created by NativeEngine).
     */
    NativeMaterial(MemorySegment materialHandle, Arena arena) {
        this.materialHandle = materialHandle;
        this.arena = arena;
    }
    
    /**
     * Update material parameters.
     */
    public void update(MaterialDesc desc) {
        checkClosed();
        try {
            MemorySegment descSegment = arena.allocate(EngineBindings.MATERIAL_DESC_LAYOUT);
            
            // Populate struct fields
            VarHandle base_r_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("base_color_r"));
            VarHandle base_g_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("base_color_g"));
            VarHandle base_b_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("base_color_b"));
            VarHandle base_a_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("base_color_a"));
            VarHandle metallic_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("metallic"));
            VarHandle roughness_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("roughness"));
            VarHandle alpha_mode_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("alpha_mode"));
            VarHandle base_tex_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("base_color_texture_id"));
            VarHandle normal_tex_handle = EngineBindings.MATERIAL_DESC_LAYOUT.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("normal_texture_id"));
            
            base_r_handle.set(descSegment, 0L, desc.baseColorR());
            base_g_handle.set(descSegment, 0L, desc.baseColorG());
            base_b_handle.set(descSegment, 0L, desc.baseColorB());
            base_a_handle.set(descSegment, 0L, desc.baseColorA());
            metallic_handle.set(descSegment, 0L, desc.metallic());
            roughness_handle.set(descSegment, 0L, desc.roughness());
            alpha_mode_handle.set(descSegment, 0L, desc.alphaMode());
            base_tex_handle.set(descSegment, 0L, desc.baseColorTextureId());
            normal_tex_handle.set(descSegment, 0L, desc.normalTextureId());
            
            int result = (int) EngineBindings.MATERIAL_UPDATE.invoke(materialHandle, descSegment);
            
            if (result != EngineBindings.ASTRAEUS_SUCCESS) {
                throw new RuntimeException("Failed to update material, error code: " + result);
            }
            
        } catch (Throwable e) {
            throw new RuntimeException("Failed to update material", e);
        }
    }
    
    @Override
    public void close() {
        if (closed) {
            return;
        }
        
        try {
            EngineBindings.MATERIAL_DESTROY.invoke(materialHandle);
            materialHandle = null;
            closed = true;
        } catch (Throwable e) {
            throw new RuntimeException("Failed to destroy material", e);
        }
    }
    
    private void checkClosed() {
        if (closed) {
            throw new IllegalStateException("Material has been closed");
        }
    }
    
    /**
     * Get the material handle (for internal use).
     */
    MemorySegment getHandle() {
        checkClosed();
        return materialHandle;
    }
    
    /**
     * Material descriptor record (immutable, PBR parameters).
     */
    public record MaterialDesc(
        float baseColorR, float baseColorG, float baseColorB, float baseColorA,
        float metallic, float roughness,
        int alphaMode,
        int baseColorTextureId, int normalTextureId
    ) {
        /**
         * Create with default opaque white material.
         */
        public static MaterialDesc defaults() {
            return new MaterialDesc(
                1.0f, 1.0f, 1.0f, 1.0f,  // white
                0.0f,                    // non-metallic
                0.5f,                    // medium roughness
                EngineBindings.ALPHA_MODE_OPAQUE,
                0, 0                     // no textures
            );
        }
        
        /**
         * Create with custom color.
         */
        public static MaterialDesc ofColor(float r, float g, float b, float a) {
            return new MaterialDesc(r, g, b, a, 0.0f, 0.5f, 
                                   EngineBindings.ALPHA_MODE_OPAQUE, 0, 0);
        }
        
        /**
         * Create with metallic and roughness.
         */
        public static MaterialDesc ofPBR(float r, float g, float b, 
                                        float metallic, float roughness) {
            return new MaterialDesc(r, g, b, 1.0f, metallic, roughness,
                                   EngineBindings.ALPHA_MODE_OPAQUE, 0, 0);
        }
    }
}
