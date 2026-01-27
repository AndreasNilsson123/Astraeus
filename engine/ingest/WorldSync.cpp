#include "WorldSync.hpp"
#include "../scene/World.hpp"
#include <iostream>

namespace astraeus {

WorldSync::WorldSync(World* world)
    : world_(world)
    , is_initialized_(false)
    , entities_created_(0)
    , entities_updated_(0)
    , entities_deleted_(0)
{
}

WorldSync::~WorldSync() {
    shutdown();
}

bool WorldSync::initialize() {
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

void WorldSync::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    active_entities_.clear();
    is_initialized_ = false;
    std::cout << "[WorldSync] Shutdown" << std::endl;
}

void WorldSync::apply_snapshot(const SnapshotView& snapshot) {
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
        
        // Check if entity needs creation
        bool is_new = (active_entities_.find(entity_id) == active_entities_.end());
        
        if (is_new) {
            // Create entity in World if it doesn't exist yet
            // Note: We assume entity_id from snapshot matches World entity_id
            // In production, you might need a mapping table
            
            // For now, we'll update the transform and components
            // The entity should already exist or we create it implicitly
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
        
        // Update position for trail (uses apply_entity_snapshot convenience method)
        world_->apply_entity_snapshot(entity_id, entity->pos_x, entity->pos_y, entity->pos_z);
        
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

void WorldSync::reset_stats() {
    entities_created_ = 0;
    entities_updated_ = 0;
    entities_deleted_ = 0;
}

} // namespace astraeus
