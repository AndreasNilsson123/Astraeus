#include "BVH.hpp"
#include <algorithm>
#include <cmath>


namespace astraeus::spatial {

namespace {
    // Maximum entities per leaf node
    constexpr uint32_t MAX_LEAF_SIZE = 4;
    
    // Maximum tree depth
    constexpr uint32_t MAX_DEPTH = 32;
}

void BVH::build(const std::vector<Entry>& entries) {
    clear();
    
    if (entries.empty()) {
        built_ = false;
        return;
    }

    entries_ = entries;
    entity_indices_.resize(entries.size());
    for (uint32_t i = 0; i < entries.size(); ++i) {
        entity_indices_[i] = i;
    }

    // Reserve space for nodes (conservative estimate)
    nodes_.reserve(entries.size() * 2);

    // Build BVH recursively
    build_recursive(entity_indices_.data(), static_cast<uint32_t>(entity_indices_.size()), 0);

    built_ = true;
}

void BVH::clear() {
    nodes_.clear();
    entity_indices_.clear();
    entries_.clear();
    built_ = false;
}

uint32_t BVH::build_recursive(uint32_t* entity_indices, uint32_t count, uint32_t depth) {
    uint32_t node_idx = static_cast<uint32_t>(nodes_.size());
    nodes_.emplace_back();
    Node& node = nodes_[node_idx];

    // Compute bounds for this node
    node.bounds = compute_bounds(entity_indices, count);

    // Create leaf if we've reached max depth or small enough
    if (depth >= MAX_DEPTH || count <= MAX_LEAF_SIZE) {
        node.left_child = 0;
        node.right_child = count;
        node.first_entity = static_cast<uint32_t>(entity_indices - entity_indices_.data());
        return node_idx;
    }

    // Find best split
    float split_pos;
    int axis = find_best_split(entity_indices, count, split_pos);

    // Partition entities along split axis
    uint32_t mid_point = 0;
    for (uint32_t i = 0; i < count; ++i) {
        const Entry& entry = entries_[entity_indices[i]];
        float center_coord = 0.0f;
        
        switch (axis) {
            case 0: center_coord = (entry.bounds.min_x + entry.bounds.max_x) * 0.5f; break;
            case 1: center_coord = (entry.bounds.min_y + entry.bounds.max_y) * 0.5f; break;
            case 2: center_coord = (entry.bounds.min_z + entry.bounds.max_z) * 0.5f; break;
        }

        if (center_coord < split_pos) {
            std::swap(entity_indices[mid_point], entity_indices[i]);
            ++mid_point;
        }
    }

    // Ensure split is valid
    if (mid_point == 0 || mid_point == count) {
        mid_point = count / 2;
    }

    // Build child nodes
    uint32_t left = build_recursive(entity_indices, mid_point, depth + 1);
    uint32_t right = build_recursive(entity_indices + mid_point, count - mid_point, depth + 1);

    // Update node with child indices (node may have been reallocated)
    nodes_[node_idx].left_child = left;
    nodes_[node_idx].right_child = right;
    nodes_[node_idx].first_entity = 0;

    return node_idx;
}

AABB BVH::compute_bounds(const uint32_t* entity_indices, uint32_t count) const {
    if (count == 0) {
        return AABB();
    }

    AABB bounds = entries_[entity_indices[0]].bounds;
    for (uint32_t i = 1; i < count; ++i) {
        bounds.expand_to_include(entries_[entity_indices[i]].bounds);
    }
    return bounds;
}

int BVH::find_best_split(const uint32_t* entity_indices, uint32_t count, float& out_split_pos) const {
    // Simple midpoint split along longest axis
    AABB bounds = compute_bounds(entity_indices, count);
    
    float dx = bounds.max_x - bounds.min_x;
    float dy = bounds.max_y - bounds.min_y;
    float dz = bounds.max_z - bounds.min_z;

    int axis = 0;
    if (dy > dx && dy > dz) axis = 1;
    else if (dz > dx && dz > dy) axis = 2;

    switch (axis) {
        case 0: out_split_pos = (bounds.min_x + bounds.max_x) * 0.5f; break;
        case 1: out_split_pos = (bounds.min_y + bounds.max_y) * 0.5f; break;
        case 2: out_split_pos = (bounds.min_z + bounds.max_z) * 0.5f; break;
    }

    return axis;
}

void BVH::raycast(const Ray& ray, float max_distance, std::vector<RayHit>& out_hits) const {
    out_hits.clear();

    if (!built_ || nodes_.empty()) {
        return;
    }

    raycast_recursive(0, ray, 0.0f, max_distance, out_hits);

    // Sort hits by distance
    std::sort(out_hits.begin(), out_hits.end());
}

void BVH::raycast_recursive(uint32_t node_idx, const Ray& ray, float t_min, float t_max, std::vector<RayHit>& hits) const {
    if (node_idx >= nodes_.size()) return;

    const Node& node = nodes_[node_idx];

    // Test ray against node bounds
    float t_hit;
    if (!ray.intersects_aabb(node.bounds, t_min, t_max, t_hit)) {
        return;
    }

    if (node.is_leaf()) {
        // Test ray against all entities in leaf
        uint32_t count = node.right_child;
        for (uint32_t i = 0; i < count; ++i) {
            uint32_t entry_idx = entity_indices_[node.first_entity + i];
            const Entry& entry = entries_[entry_idx];

            float dist;
            if (ray.intersects_aabb(entry.bounds, t_min, t_max, dist)) {
                hits.emplace_back(entry.entity_id, dist);
            }
        }
    } else {
        // Recurse into child nodes
        raycast_recursive(node.left_child, ray, t_min, t_max, hits);
        raycast_recursive(node.right_child, ray, t_min, t_max, hits);
    }
}

bool BVH::nearest(float px, float py, float pz, float max_distance, uint32_t& out_entity_id, float& out_distance) const {
    if (!built_ || nodes_.empty()) {
        return false;
    }

    float best_dist_sq = max_distance * max_distance;
    uint32_t best_entity = 0;

    nearest_recursive(0, px, py, pz, best_dist_sq, best_entity);

    if (best_entity != 0) {
        out_entity_id = best_entity;
        out_distance = std::sqrt(best_dist_sq);
        return true;
    }

    return false;
}

void BVH::nearest_recursive(uint32_t node_idx, float px, float py, float pz, float& best_dist_sq, uint32_t& best_entity) const {
    if (node_idx >= nodes_.size()) return;

    const Node& node = nodes_[node_idx];

    // Compute minimum squared distance to node bounds
    float dx = std::max(0.0f, std::max(node.bounds.min_x - px, px - node.bounds.max_x));
    float dy = std::max(0.0f, std::max(node.bounds.min_y - py, py - node.bounds.max_y));
    float dz = std::max(0.0f, std::max(node.bounds.min_z - pz, pz - node.bounds.max_z));
    float min_dist_sq = dx * dx + dy * dy + dz * dz;

    if (min_dist_sq > best_dist_sq) {
        return;
    }

    if (node.is_leaf()) {
        // Test all entities in leaf
        uint32_t count = node.right_child;
        for (uint32_t i = 0; i < count; ++i) {
            uint32_t entry_idx = entity_indices_[node.first_entity + i];
            const Entry& entry = entries_[entry_idx];

            // Compute distance to entity center
            float cx, cy, cz;
            entry.bounds.get_center(cx, cy, cz);
            float dist_sq = (cx - px) * (cx - px) + (cy - py) * (cy - py) + (cz - pz) * (cz - pz);

            if (dist_sq < best_dist_sq) {
                best_dist_sq = dist_sq;
                best_entity = entry.entity_id;
            }
        }
    } else {
        // Recurse into child nodes
        nearest_recursive(node.left_child, px, py, pz, best_dist_sq, best_entity);
        nearest_recursive(node.right_child, px, py, pz, best_dist_sq, best_entity);
    }
}

void BVH::query_aabb(const AABB& query_box, std::vector<uint32_t>& out_entities) const {
    out_entities.clear();

    if (!built_ || nodes_.empty()) {
        return;
    }

    query_aabb_recursive(0, query_box, out_entities);
}

void BVH::query_aabb_recursive(uint32_t node_idx, const AABB& query_box, std::vector<uint32_t>& out_entities) const {
    if (node_idx >= nodes_.size()) return;

    const Node& node = nodes_[node_idx];

    // Test query box against node bounds
    if (!node.bounds.intersects(query_box)) {
        return;
    }

    if (node.is_leaf()) {
        // Test all entities in leaf
        uint32_t count = node.right_child;
        for (uint32_t i = 0; i < count; ++i) {
            uint32_t entry_idx = entity_indices_[node.first_entity + i];
            const Entry& entry = entries_[entry_idx];

            if (entry.bounds.intersects(query_box)) {
                out_entities.push_back(entry.entity_id);
            }
        }
    } else {
        // Recurse into child nodes
        query_aabb_recursive(node.left_child, query_box, out_entities);
        query_aabb_recursive(node.right_child, query_box, out_entities);
    }
}

} // namespace astraeus::spatial

