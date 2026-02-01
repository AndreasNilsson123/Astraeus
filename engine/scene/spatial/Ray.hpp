#ifndef ASTRAEUS_SPATIAL_RAY_HPP
#define ASTRAEUS_SPATIAL_RAY_HPP

#include "AABB.hpp"
#include <cmath>
#include <limits>

namespace astraeus {
namespace spatial {

/**
 * Ray for raycasting queries.
 */
struct Ray {
    float origin_x, origin_y, origin_z;
    float dir_x, dir_y, dir_z;

    Ray()
        : origin_x(0.0f), origin_y(0.0f), origin_z(0.0f)
        , dir_x(0.0f), dir_y(0.0f), dir_z(1.0f)
    {}

    Ray(float ox, float oy, float oz, float dx, float dy, float dz)
        : origin_x(ox), origin_y(oy), origin_z(oz)
        , dir_x(dx), dir_y(dy), dir_z(dz)
    {}

    /**
     * Normalize ray direction.
     */
    void normalize() {
        float len = std::sqrt(dir_x * dir_x + dir_y * dir_y + dir_z * dir_z);
        if (len > 1e-6f) {
            float inv_len = 1.0f / len;
            dir_x *= inv_len;
            dir_y *= inv_len;
            dir_z *= inv_len;
        }
    }

    /**
     * Test ray-AABB intersection using slab method.
     * Returns true if intersection occurs within [t_min, t_max].
     * On intersection, out_t contains the hit distance.
     */
    bool intersects_aabb(const AABB& aabb, float t_min, float t_max, float& out_t) const {
        float t_near = t_min;
        float t_far = t_max;

        // X axis
        if (std::abs(dir_x) > 1e-6f) {
            float t1 = (aabb.min_x - origin_x) / dir_x;
            float t2 = (aabb.max_x - origin_x) / dir_x;
            if (t1 > t2) std::swap(t1, t2);
            t_near = std::max(t_near, t1);
            t_far = std::min(t_far, t2);
            if (t_near > t_far) return false;
        } else {
            if (origin_x < aabb.min_x || origin_x > aabb.max_x) return false;
        }

        // Y axis
        if (std::abs(dir_y) > 1e-6f) {
            float t1 = (aabb.min_y - origin_y) / dir_y;
            float t2 = (aabb.max_y - origin_y) / dir_y;
            if (t1 > t2) std::swap(t1, t2);
            t_near = std::max(t_near, t1);
            t_far = std::min(t_far, t2);
            if (t_near > t_far) return false;
        } else {
            if (origin_y < aabb.min_y || origin_y > aabb.max_y) return false;
        }

        // Z axis
        if (std::abs(dir_z) > 1e-6f) {
            float t1 = (aabb.min_z - origin_z) / dir_z;
            float t2 = (aabb.max_z - origin_z) / dir_z;
            if (t1 > t2) std::swap(t1, t2);
            t_near = std::max(t_near, t1);
            t_far = std::min(t_far, t2);
            if (t_near > t_far) return false;
        } else {
            if (origin_z < aabb.min_z || origin_z > aabb.max_z) return false;
        }

        out_t = t_near;
        return true;
    }
};

} // namespace spatial
} // namespace astraeus

#endif // ASTRAEUS_SPATIAL_RAY_HPP
