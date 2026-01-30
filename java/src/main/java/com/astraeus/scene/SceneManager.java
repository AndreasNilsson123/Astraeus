package com.astraeus.scene;

import com.astraeus.native_api.NativeEngine;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;

import java.util.HashMap;
import java.util.Map;

/**
 * Scene manager that maintains client-side entity registry.
 * Provides entity creation/deletion and synchronizes transforms to engine.
 */
public class SceneManager {
    
    private final NativeEngine engine;
    private final Map<Integer, EntityData> entities;
    private final ObservableList<EntityData> observableEntities;
    
    public SceneManager(NativeEngine engine) {
        this.engine = engine;
        this.entities = new HashMap<>();
        this.observableEntities = FXCollections.observableArrayList();
    }
    
    /**
     * Create a new entity.
     * Creates the entity in the native engine and tracks it locally.
     */
    public EntityData createEntity() {
        int entityId = engine.createEntity();
        EntityData data = new EntityData(entityId);
        entities.put(entityId, data);
        observableEntities.add(data);
        
        // Set default transform in engine
        syncTransformToEngine(data);
        
        return data;
    }
    
    /**
     * Create multiple entities (for testing/batch creation).
     */
    public void createEntities(int count) {
        for (int i = 0; i < count; i++) {
            createEntity();
        }
    }
    
    /**
     * Destroy an entity.
     */
    public void destroyEntity(int entityId) {
        EntityData data = entities.remove(entityId);
        if (data != null) {
            observableEntities.remove(data);
            engine.destroyEntity(entityId);
        }
    }
    
    /**
     * Get entity data by ID.
     */
    public EntityData getEntity(int entityId) {
        return entities.get(entityId);
    }
    
    /**
     * Get all entities.
     */
    public ObservableList<EntityData> getEntities() {
        return observableEntities;
    }
    
    /**
     * Get entity count.
     */
    public int getEntityCount() {
        return entities.size();
    }
    
    /**
     * Clear all entities.
     */
    public void clearAll() {
        for (EntityData data : entities.values()) {
            engine.destroyEntity(data.getEntityId());
        }
        entities.clear();
        observableEntities.clear();
    }
    
    /**
     * Synchronize entity transform to engine.
     * Call this after modifying transform properties.
     */
    public void syncTransformToEngine(EntityData entity) {
        if (engine == null || !engine.isValid()) {
            return;
        }
        
        try {
            // Use FFM bindings to set transform
            updateEntityTransform(
                entity.getEntityId(),
                (float) entity.getPosX(),
                (float) entity.getPosY(),
                (float) entity.getPosZ(),
                (float) entity.getRotX(),
                (float) entity.getRotY(),
                (float) entity.getRotZ(),
                (float) entity.getScaleX(),
                (float) entity.getScaleY(),
                (float) entity.getScaleZ()
            );
        } catch (Exception e) {
            System.err.println("Failed to sync transform for entity " + entity.getEntityId() + ": " + e.getMessage());
        }
    }
    
    /**
     * Synchronize entity visibility to engine.
     */
    public void syncVisibilityToEngine(EntityData entity) {
        if (engine == null || !engine.isValid()) {
            return;
        }
        
        try {
            updateEntityRenderable(entity.getEntityId(), entity.isVisible());
        } catch (Exception e) {
            System.err.println("Failed to sync visibility for entity " + entity.getEntityId() + ": " + e.getMessage());
        }
    }
    
    /**
     * Synchronize entity color to engine.
     */
    public void syncColorToEngine(EntityData entity) {
        if (engine == null || !engine.isValid()) {
            return;
        }
        
        try {
            updateEntityColor(
                entity.getEntityId(),
                (float) entity.getColorR(),
                (float) entity.getColorG(),
                (float) entity.getColorB(),
                (float) entity.getColorA()
            );
        } catch (Exception e) {
            System.err.println("Failed to sync color for entity " + entity.getEntityId() + ": " + e.getMessage());
        }
    }
    
    /**
     * Synchronize entity trail to engine.
     */
    public void syncTrailToEngine(EntityData entity) {
        if (engine == null || !engine.isValid()) {
            return;
        }
        
        try {
            updateEntityTrail(entity.getEntityId(), entity.getTrailMaxPoints());
        } catch (Exception e) {
            System.err.println("Failed to sync trail for entity " + entity.getEntityId() + ": " + e.getMessage());
        }
    }
    
    // ==================== Native Engine Wrappers ====================
    
    private void updateEntityTransform(int entityId, float posX, float posY, float posZ,
                                      float rotX, float rotY, float rotZ,
                                      float scaleX, float scaleY, float scaleZ) {
        engine.setEntityTransform(entityId, posX, posY, posZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ);
    }
    
    private void updateEntityRenderable(int entityId, boolean visible) {
        engine.setEntityRenderable(entityId, visible);
    }
    
    private void updateEntityColor(int entityId, float r, float g, float b, float a) {
        engine.setEntityColor(entityId, r, g, b, a);
    }
    
    private void updateEntityTrail(int entityId, int maxPoints) {
        engine.setEntityTrail(entityId, maxPoints);
    }
}
