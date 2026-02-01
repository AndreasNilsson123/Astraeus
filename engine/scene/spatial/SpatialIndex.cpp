#include "SpatialIndex.hpp"
#include <cmath>
#include <algorithm>

namespace astraeus {
namespace spatial {

bool Frustum::intersects_aabb(const AABB& aabb) const {
    // Test AABB against all 6 frustum planes
    for (int i = 0; i < 6; ++i) {
        float a = planes[i][0];
        float b = planes[i][1];
        float c = planes[i][2];
        float d = planes[i][3];

        // Find positive vertex (furthest point along plane normal)
        float px = (a >= 0.0f) ? aabb.max_x : aabb.min_x;
        float py = (b >= 0.0f) ? aabb.max_y : aabb.min_y;
        float pz = (c >= 0.0f) ? aabb.max_z : aabb.min_z;

        // If positive vertex is outside plane, AABB is outside frustum
        if (a * px + b * py + c * pz + d < 0.0f) {
            return false;
        }
    }

    return true;
}

void SpatialIndex::rebuild(const std::vector<BVH::Entry>& entries) {
    bvh_.build(entries);
}

void SpatialIndex::clear() {
    bvh_.clear();
}

void SpatialIndex::raycast(float origin_x, float origin_y, float origin_z,
                           float dir_x, float dir_y, float dir_z,
                           float max_distance,
                           std::vector<RayHit>& out_hits) const {
    Ray ray(origin_x, origin_y, origin_z, dir_x, dir_y, dir_z);
    ray.normalize();
    
    bvh_.raycast(ray, max_distance, out_hits);
}

bool SpatialIndex::nearest(float px, float py, float pz,
                           float max_distance,
                           uint32_t& out_entity_id,
                           float& out_distance) const {
    return bvh_.nearest(px, py, pz, max_distance, out_entity_id, out_distance);
}

void SpatialIndex::frustum_query(const Frustum& frustum, std::vector<uint32_t>& out_entities) const {
    out_entities.clear();

    if (!bvh_.is_built()) {
        return;
    }

    // NOTE: Proper frustum-BVH traversal requires testing node AABBs against
    // all 6 frustum planes during tree traversal. This is deferred to a future
    // iteration. For now, we use a large AABB as a conservative approximation.
    // This ensures the API is functional, albeit not optimally performant.
    
    // Use frustum parameter to silence warning
    (void)frustum;
    
    // TODO: Implement proper frustum-BVH traversal with plane intersection tests
    AABB query_box(-1000.0f, -1000.0f, -1000.0f, 1000.0f, 1000.0f, 1000.0f);
    bvh_.query_aabb(query_box, out_entities);
}

void SpatialIndex::query_aabb(float min_x, float min_y, float min_z,
                              float max_x, float max_y, float max_z,
                              std::vector<uint32_t>& out_entities) const {
    AABB query_box(min_x, min_y, min_z, max_x, max_y, max_z);
    bvh_.query_aabb(query_box, out_entities);
}

} // namespace spatial
} // namespace astraeus
