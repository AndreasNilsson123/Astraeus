#include "World.hpp"
#include <iostream>
#include <algorithm>

namespace astraeus {

World::World()
    : next_entity_id_(1) // Start at 1, 0 is reserved for "null"
    , is_initialized_(false)
{
}

World::~World() {
    shutdown();
}

bool World::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[World] Initializing world" << std::endl;
    
    is_initialized_ = true;
    return true;
}

void World::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[World] Shutting down" << std::endl;
    
    transforms_.clear();
    active_entities_.clear();
    next_entity_id_ = 1;
    
    is_initialized_ = false;
}

uint32_t World::create_entity() {
    uint32_t entity_id = next_entity_id_++;
    
    // Initialize with default transform
    transforms_[entity_id] = Transform();
    active_entities_.push_back(entity_id);
    
    return entity_id;
}

void World::destroy_entity(uint32_t entity_id) {
    if (entity_id == 0) {
        return;
    }

    // Remove transform
    transforms_.erase(entity_id);
    
    // Remove from active list
    auto it = std::find(active_entities_.begin(), active_entities_.end(), entity_id);
    if (it != active_entities_.end()) {
        active_entities_.erase(it);
    }
}

uint32_t World::get_entity_count() const {
    return static_cast<uint32_t>(active_entities_.size());
}

void World::set_entity_transform(uint32_t entity_id,
                                 float pos_x, float pos_y, float pos_z,
                                 float rot_x, float rot_y, float rot_z,
                                 float scale_x, float scale_y, float scale_z) {
    if (entity_id == 0) {
        return;
    }

    auto it = transforms_.find(entity_id);
    if (it != transforms_.end()) {
        Transform& t = it->second;
        t.pos_x = pos_x; t.pos_y = pos_y; t.pos_z = pos_z;
        t.rot_x = rot_x; t.rot_y = rot_y; t.rot_z = rot_z;
        t.scale_x = scale_x; t.scale_y = scale_y; t.scale_z = scale_z;
    }
}

const Transform* World::get_entity_transform(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }

    auto it = transforms_.find(entity_id);
    if (it != transforms_.end()) {
        return &it->second;
    }
    return nullptr;
}

void World::set_camera(float eye_x, float eye_y, float eye_z,
                      float target_x, float target_y, float target_z,
                      float up_x, float up_y, float up_z) {
    camera_.set_view(eye_x, eye_y, eye_z,
                     target_x, target_y, target_z,
                     up_x, up_y, up_z);
}

void World::set_camera_projection(float fov_degrees, float near_plane, float far_plane) {
    // Aspect ratio will be updated in update_camera()
    camera_.set_projection(fov_degrees, 1.0f, near_plane, far_plane);
}

void World::update_camera(float aspect_ratio) {
    camera_.update_matrices(aspect_ratio);
}

} // namespace astraeus
