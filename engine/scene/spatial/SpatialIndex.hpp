#ifndef ASTRAEUS_SPATIAL_SPATIALINDEX_HPP
#define ASTRAEUS_SPATIAL_SPATIALINDEX_HPP

#include "BVH.hpp"
#include "Ray.hpp"
#include "AABB.hpp"
#include <vector>
#include <cstdint>

namespace astraeus {
namespace spatial {

/**
 * Frustum represented as 6 planes for culling queries.
 */
struct Frustum {
    // Each plane as (a, b, c, d) where ax + by + cz + d = 0
    float planes[6][4];

    /**
     * Test if AABB intersects frustum.
     */
    bool intersects_aabb(const AABB& aabb) const;
};

/**
 * SpatialIndex provides CPU-side spatial query capabilities.
 * Wraps a BVH for efficient queries on large entity counts.
 */
class SpatialIndex {
public:
    SpatialIndex() = default;
    ~SpatialIndex() = default;

    /**
     * Rebuild spatial index from entity bounds.
     * Call this whenever entities move or are added/removed.
     */
    void rebuild(const std::vector<BVH::Entry>& entries);

    /**
     * Clear spatial index.
     */
    void clear();

    /**
     * Raycast query: find all entities intersecting a ray.
     * Results are sorted by distance (closest first).
     * 
     * @param origin_x Ray origin X coordinate
     * @param origin_y Ray origin Y coordinate
     * @param origin_z Ray origin Z coordinate
     * @param dir_x Ray direction X (will be normalized)
     * @param dir_y Ray direction Y (will be normalized)
     * @param dir_z Ray direction Z (will be normalized)
     * @param max_distance Maximum ray distance
     * @param out_hits Output vector of hits (entity_id, distance)
     */
    void raycast(float origin_x, float origin_y, float origin_z,
                 float dir_x, float dir_y, float dir_z,
                 float max_distance,
                 std::vector<RayHit>& out_hits) const;

    /**
     * Nearest query: find closest entity to a point.
     * 
     * @param px Point X coordinate
     * @param py Point Y coordinate
     * @param pz Point Z coordinate
     * @param max_distance Maximum search distance
     * @param out_entity_id Output entity ID (0 if none found)
     * @param out_distance Output distance to entity
     * @return true if entity found, false otherwise
     */
    bool nearest(float px, float py, float pz,
                 float max_distance,
                 uint32_t& out_entity_id,
                 float& out_distance) const;

    /**
     * Frustum query: find all entities within view frustum.
     * 
     * NOTE: Current implementation uses AABB approximation as a placeholder.
     * Proper frustum-BVH traversal is deferred to future iteration.
     * 
     * @param frustum View frustum (6 planes)
     * @param out_entities Output vector of entity IDs
     */
    void frustum_query(const Frustum& frustum, std::vector<uint32_t>& out_entities) const;

    /**
     * AABB query: find all entities overlapping an AABB.
     * 
     * @param min_x AABB minimum X
     * @param min_y AABB minimum Y
     * @param min_z AABB minimum Z
     * @param max_x AABB maximum X
     * @param max_y AABB maximum Y
     * @param max_z AABB maximum Z
     * @param out_entities Output vector of entity IDs
     */
    void query_aabb(float min_x, float min_y, float min_z,
                    float max_x, float max_y, float max_z,
                    std::vector<uint32_t>& out_entities) const;

    /**
     * Check if spatial index is built and ready for queries.
     */
    bool is_ready() const { return bvh_.is_built(); }

private:
    BVH bvh_;
};

} // namespace spatial
} // namespace astraeus

#endif // ASTRAEUS_SPATIAL_SPATIALINDEX_HPP
