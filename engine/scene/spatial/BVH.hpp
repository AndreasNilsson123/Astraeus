#ifndef ASTRAEUS_SPATIAL_BVH_HPP
#define ASTRAEUS_SPATIAL_BVH_HPP

#include "AABB.hpp"
#include "Ray.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>

namespace astraeus {
namespace spatial {

/**
 * Hit result from a raycast query.
 */
struct RayHit {
    uint32_t entity_id;
    float distance;

    RayHit() : entity_id(0), distance(std::numeric_limits<float>::max()) {}
    RayHit(uint32_t eid, float dist) : entity_id(eid), distance(dist) {}

    bool operator<(const RayHit& other) const {
        return distance < other.distance;
    }
};

/**
 * BVH (Bounding Volume Hierarchy) for spatial queries.
 * Uses binary tree structure with midpoint splitting along longest axis.
 */
class BVH {
public:
    /**
     * Entry in the BVH - associates entity with its AABB.
     */
    struct Entry {
        uint32_t entity_id;
        AABB bounds;

        Entry() : entity_id(0) {}
        Entry(uint32_t eid, const AABB& aabb) : entity_id(eid), bounds(aabb) {}
    };

private:
    /**
     * BVH node structure.
     */
    struct Node {
        AABB bounds;
        uint32_t left_child;   // Index to left child (0 = leaf)
        uint32_t right_child;  // Index to right child or entity count if leaf
        uint32_t first_entity; // Index to first entity in leaf

        bool is_leaf() const { return left_child == 0; }
    };

    std::vector<Node> nodes_;
    std::vector<uint32_t> entity_indices_;
    std::vector<Entry> entries_;
    bool built_;

public:
    BVH() : built_(false) {}

    /**
     * Build BVH from entity entries.
     */
    void build(const std::vector<Entry>& entries);

    /**
     * Clear BVH.
     */
    void clear();

    /**
     * Query all entities intersecting a ray.
     * Results are sorted by distance (closest first).
     */
    void raycast(const Ray& ray, float max_distance, std::vector<RayHit>& out_hits) const;

    /**
     * Query nearest entity to a point.
     */
    bool nearest(float px, float py, float pz, float max_distance, uint32_t& out_entity_id, float& out_distance) const;

    /**
     * Query all entities within an AABB (frustum-like query).
     */
    void query_aabb(const AABB& query_box, std::vector<uint32_t>& out_entities) const;

    /**
     * Check if BVH is built.
     */
    bool is_built() const { return built_; }

private:
    // Build BVH recursively using SAH
    uint32_t build_recursive(uint32_t* entity_indices, uint32_t count, uint32_t depth);

    // Compute AABB for a range of entities
    AABB compute_bounds(const uint32_t* entity_indices, uint32_t count) const;

    // Find best split axis and position using SAH
    int find_best_split(const uint32_t* entity_indices, uint32_t count, float& out_split_pos) const;

    // Recursive raycast traversal
    void raycast_recursive(uint32_t node_idx, const Ray& ray, float t_min, float t_max, std::vector<RayHit>& hits) const;

    // Recursive nearest traversal
    void nearest_recursive(uint32_t node_idx, float px, float py, float pz, float& best_dist_sq, uint32_t& best_entity) const;

    // Recursive AABB query traversal
    void query_aabb_recursive(uint32_t node_idx, const AABB& query_box, std::vector<uint32_t>& out_entities) const;
};

} // namespace spatial
} // namespace astraeus

#endif // ASTRAEUS_SPATIAL_BVH_HPP
