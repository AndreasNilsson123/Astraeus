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
    renderables_.clear();
    colors_.clear();
    trails_.clear();
    labels_.clear();
    active_entities_.clear();
    renderable_entities_cache_.clear();
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

    // Remove all components
    transforms_.erase(entity_id);
    renderables_.erase(entity_id);
    colors_.erase(entity_id);
    trails_.erase(entity_id);
    labels_.erase(entity_id);
    
    // Remove from active list
    auto it = std::find(active_entities_.begin(), active_entities_.end(), entity_id);
    if (it != active_entities_.end()) {
        active_entities_.erase(it);
    }
    
    // Remove from renderable cache
    auto rit = std::find(renderable_entities_cache_.begin(), renderable_entities_cache_.end(), entity_id);
    if (rit != renderable_entities_cache_.end()) {
        renderable_entities_cache_.erase(rit);
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

void World::set_entity_renderable(uint32_t entity_id, bool visible) {
    if (entity_id == 0) {
        return;
    }
    
    auto it = renderables_.find(entity_id);
    if (it != renderables_.end()) {
        // Entity already has renderable component
        bool was_visible = it->second.visible;
        it->second.visible = visible;
        
        // Update cache based on visibility change
        if (was_visible && !visible) {
            // Remove from cache
            auto cache_it = std::find(renderable_entities_cache_.begin(), 
                                     renderable_entities_cache_.end(), 
                                     entity_id);
            if (cache_it != renderable_entities_cache_.end()) {
                renderable_entities_cache_.erase(cache_it);
            }
        } else if (!was_visible && visible) {
            // Add to cache
            renderable_entities_cache_.push_back(entity_id);
        }
    } else {
        // New renderable component
        Renderable r;
        r.visible = visible;
        renderables_[entity_id] = r;
        if (visible) {
            renderable_entities_cache_.push_back(entity_id);
        }
    }
}

const Renderable* World::get_entity_renderable(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = renderables_.find(entity_id);
    if (it != renderables_.end()) {
        return &it->second;
    }
    return nullptr;
}

void World::set_entity_color(uint32_t entity_id, float r, float g, float b, float a) {
    if (entity_id == 0) {
        return;
    }
    
    colors_[entity_id] = Color(r, g, b, a);
}

const Color* World::get_entity_color(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = colors_.find(entity_id);
    if (it != colors_.end()) {
        return &it->second;
    }
    return nullptr;
}

void World::set_entity_trail(uint32_t entity_id, uint32_t max_points) {
    if (entity_id == 0) {
        return;
    }
    
    trails_[entity_id] = TrackTrail(max_points);
}

const TrackTrail* World::get_entity_trail(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = trails_.find(entity_id);
    if (it != trails_.end()) {
        return &it->second;
    }
    return nullptr;
}

void World::add_entity_trail_point(uint32_t entity_id, float x, float y, float z) {
    if (entity_id == 0) {
        return;
    }
    
    auto it = trails_.find(entity_id);
    if (it != trails_.end()) {
        it->second.add_point(x, y, z);
    }
}

void World::set_entity_label(uint32_t entity_id, uint32_t label_id) {
    if (entity_id == 0) {
        return;
    }
    
    labels_[entity_id] = LabelRef(label_id);
}

const LabelRef* World::get_entity_label(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = labels_.find(entity_id);
    if (it != labels_.end()) {
        return &it->second;
    }
    return nullptr;
}

void World::apply_entity_snapshot(uint32_t entity_id, float pos_x, float pos_y, float pos_z) {
    if (entity_id == 0) {
        return;
    }
    
    // Update transform
    auto trans_it = transforms_.find(entity_id);
    if (trans_it != transforms_.end()) {
        trans_it->second.pos_x = pos_x;
        trans_it->second.pos_y = pos_y;
        trans_it->second.pos_z = pos_z;
        
        // Update trail if it exists
        auto trail_it = trails_.find(entity_id);
        if (trail_it != trails_.end()) {
            trail_it->second.add_point(pos_x, pos_y, pos_z);
        }
    }
}

const std::vector<uint32_t>& World::get_renderable_entities() const {
    return renderable_entities_cache_;
}

} // namespace astraeus
