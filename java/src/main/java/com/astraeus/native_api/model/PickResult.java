package com.astraeus.native_api.model;

import com.astraeus.native_api.EngineBindings;

import java.lang.foreign.MemoryLayout;
import java.lang.foreign.MemorySegment;
import java.lang.invoke.VarHandle;

/**
 * View wrapper for PickResult struct from native engine.
 * Provides safe access to picking data returned from the C API.
 * 
 * This class is immutable and represents a snapshot of a pick operation result.
 */
public class PickResult {
    
    private final int entityId;
    private final float depth;
    private final float worldX;
    private final float worldY;
    private final float worldZ;
    private final boolean hit;
    
    /**
     * Create a PickResult from a native PickResult struct.
     * 
     * @param structSegment Memory segment containing the PickResult struct
     */
    public PickResult(MemorySegment structSegment) {
        // Get field handles for safe access
        VarHandle entityIdHandle = EngineBindings.PICK_RESULT_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("entity_id"));
        VarHandle depthHandle = EngineBindings.PICK_RESULT_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("depth"));
        VarHandle worldXHandle = EngineBindings.PICK_RESULT_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("world_x"));
        VarHandle worldYHandle = EngineBindings.PICK_RESULT_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("world_y"));
        VarHandle worldZHandle = EngineBindings.PICK_RESULT_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("world_z"));
        VarHandle hitHandle = EngineBindings.PICK_RESULT_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("hit"));
        
        // Read values from struct
        this.entityId = (int) entityIdHandle.get(structSegment, 0L);
        this.depth = (float) depthHandle.get(structSegment, 0L);
        this.worldX = (float) worldXHandle.get(structSegment, 0L);
        this.worldY = (float) worldYHandle.get(structSegment, 0L);
        this.worldZ = (float) worldZHandle.get(structSegment, 0L);
        this.hit = (boolean) hitHandle.get(structSegment, 0L);
    }
    
    /**
     * Get the entity ID that was picked.
     * @return Entity ID (0 if no hit)
     */
    public int getEntityId() {
        return entityId;
    }
    
    /**
     * Get the depth value at the picked location.
     * @return Depth value in normalized device coordinates [0, 1]
     */
    public float getDepth() {
        return depth;
    }
    
    /**
     * Get the world X coordinate of the picked point.
     * @return World X coordinate
     */
    public float getWorldX() {
        return worldX;
    }
    
    /**
     * Get the world Y coordinate of the picked point.
     * @return World Y coordinate
     */
    public float getWorldY() {
        return worldY;
    }
    
    /**
     * Get the world Z coordinate of the picked point.
     * @return World Z coordinate
     */
    public float getWorldZ() {
        return worldZ;
    }
    
    /**
     * Check if the pick operation hit an entity.
     * @return true if an entity was hit, false otherwise
     */
    public boolean isHit() {
        return hit;
    }
    
    /**
     * Check if this pick result represents a valid entity selection.
     * @return true if hit and entity ID is valid (non-zero)
     */
    public boolean hasValidEntity() {
        return hit && entityId != 0;
    }
    
    @Override
    public String toString() {
        if (!hit) {
            return "PickResult{no hit}";
        }
        return String.format("PickResult{entityId=%d, depth=%.3f, world=(%.2f, %.2f, %.2f)}", 
                           entityId, depth, worldX, worldY, worldZ);
    }
}
