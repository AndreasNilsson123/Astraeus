#ifndef ASTRAEUS_WORLD_SYNC_HPP
#define ASTRAEUS_WORLD_SYNC_HPP

#include "SnapshotView.h"
#include "../scene/World.hpp"
#include <cstdint>
#include <unordered_set>
#include <vector>
#include <iostream>

namespace astraeus {

/**
 * WorldSync applies SnapshotView data to the World scene graph.
 * Handles entity creation, updates, and deletion based on snapshot data.
 */
class WorldSync {
public:
    explicit WorldSync(World* world);
    ~WorldSync();
    
    /**
     * Initialize.
     */
    bool initialize();
    
    /**
     * Shutdown.
     */
    void shutdown();
    
    /**
     * Apply a snapshot to the world.
     * Creates/updates/deletes entities as needed.
     */
    void apply_snapshot(const SnapshotView& snapshot);
    
    /**
     * Get statistics.
     */
    uint32_t get_entities_created() const { return entities_created_; }
    uint32_t get_entities_updated() const { return entities_updated_; }
    uint32_t get_entities_deleted() const { return entities_deleted_; }
    
    /**
     * Reset statistics.
     */
    void reset_stats();
    
private:
    World* world_;
    bool is_initialized_;
    
    // Track which entities exist in World (for deletion detection)
    std::unordered_set<uint32_t> active_entities_;
    
    // Statistics
    uint32_t entities_created_;
    uint32_t entities_updated_;
    uint32_t entities_deleted_;
};

// Inline implementations

inline WorldSync::WorldSync(World* world)
    : world_(world)
    , is_initialized_(false)
    , entities_created_(0)
    , entities_updated_(0)
    , entities_deleted_(0)
{
}

inline WorldSync::~WorldSync() {
    shutdown();
}

inline bool WorldSync::initialize() {
    if (is_initialized_) {
        return true;
    }
    
    if (!world_) {
        std::cerr << "[WorldSync] Cannot initialize with null World" << std::endl;
        return false;
    }
    
    is_initialized_ = true;
    std::cout << "[WorldSync] Initialized" << std::endl;
    return true;
}

inline void WorldSync::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    active_entities_.clear();
    is_initialized_ = false;
    std::cout << "[WorldSync] Shutdown" << std::endl;
}

inline void WorldSync::apply_snapshot(const SnapshotView& snapshot) {
    if (!is_initialized_ || !snapshot.is_valid()) {
        return;
    }
    
    // Track entities in this snapshot
    std::unordered_set<uint32_t> snapshot_entities;
    
    // Process all entities in snapshot
    for (uint32_t i = 0; i < snapshot.get_entity_count(); i++) {
        const EntitySnapshot* entity = snapshot.get_entity(i);
        if (!entity || !entity->active) {
            continue;
        }
        
        uint32_t entity_id = entity->entity_id;
        snapshot_entities.insert(entity_id);
        
        // Ensure entity exists in World (creates if needed)
        bool was_created = world_->ensure_entity(entity_id);
        
        // Track for statistics
        bool is_new = (active_entities_.find(entity_id) == active_entities_.end());
        if (is_new) {
            active_entities_.insert(entity_id);
            entities_created_++;
        }
        
        // Update transform
        world_->set_entity_transform(
            entity_id,
            entity->pos_x, entity->pos_y, entity->pos_z,
            entity->rot_x, entity->rot_y, entity->rot_z,
            entity->scale_x, entity->scale_y, entity->scale_z
        );
        
        // Update color
        world_->set_entity_color(
            entity_id,
            entity->color_r, entity->color_g, entity->color_b, entity->color_a
        );
        
        // Update renderable (make visible)
        world_->set_entity_renderable(entity_id, true);
        
        if (!is_new) {
            entities_updated_++;
        }
    }
    
    // Delete entities that are no longer in snapshot
    std::vector<uint32_t> to_delete;
    for (uint32_t entity_id : active_entities_) {
        if (snapshot_entities.find(entity_id) == snapshot_entities.end()) {
            to_delete.push_back(entity_id);
        }
    }
    
    for (uint32_t entity_id : to_delete) {
        world_->destroy_entity(entity_id);
        active_entities_.erase(entity_id);
        entities_deleted_++;
    }
}

inline void WorldSync::reset_stats() {
    entities_created_ = 0;
    entities_updated_ = 0;
    entities_deleted_ = 0;
}

} // namespace astraeus

#endif // ASTRAEUS_WORLD_SYNC_HPP
