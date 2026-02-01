#ifndef ASTRAEUS_SPATIAL_AABB_HPP
#define ASTRAEUS_SPATIAL_AABB_HPP

#include <cmath>
#include <algorithm>

namespace astraeus {
namespace spatial {

/**
 * Axis-Aligned Bounding Box (AABB).
 * Used for spatial queries and broad-phase collision detection.
 */
struct AABB {
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;

    AABB() 
        : min_x(0.0f), min_y(0.0f), min_z(0.0f)
        , max_x(0.0f), max_y(0.0f), max_z(0.0f)
    {}

    AABB(float min_x_, float min_y_, float min_z_,
         float max_x_, float max_y_, float max_z_)
        : min_x(min_x_), min_y(min_y_), min_z(min_z_)
        , max_x(max_x_), max_y(max_y_), max_z(max_z_)
    {}

    /**
     * Get center point of AABB.
     */
    void get_center(float& out_x, float& out_y, float& out_z) const {
        out_x = (min_x + max_x) * 0.5f;
        out_y = (min_y + max_y) * 0.5f;
        out_z = (min_z + max_z) * 0.5f;
    }

    /**
     * Get extents (half-dimensions) of AABB.
     */
    void get_extents(float& out_x, float& out_y, float& out_z) const {
        out_x = (max_x - min_x) * 0.5f;
        out_y = (max_y - min_y) * 0.5f;
        out_z = (max_z - min_z) * 0.5f;
    }

    /**
     * Get surface area of AABB (for SAH calculations).
     */
    float surface_area() const {
        float dx = max_x - min_x;
        float dy = max_y - min_y;
        float dz = max_z - min_z;
        return 2.0f * (dx * dy + dy * dz + dz * dx);
    }

    /**
     * Check if AABB contains a point.
     */
    bool contains_point(float x, float y, float z) const {
        return x >= min_x && x <= max_x &&
               y >= min_y && y <= max_y &&
               z >= min_z && z <= max_z;
    }

    /**
     * Check if this AABB intersects another AABB.
     */
    bool intersects(const AABB& other) const {
        return (min_x <= other.max_x && max_x >= other.min_x) &&
               (min_y <= other.max_y && max_y >= other.min_y) &&
               (min_z <= other.max_z && max_z >= other.min_z);
    }

    /**
     * Expand AABB to include a point.
     */
    void expand_to_include(float x, float y, float z) {
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        min_z = std::min(min_z, z);
        max_x = std::max(max_x, x);
        max_y = std::max(max_y, y);
        max_z = std::max(max_z, z);
    }

    /**
     * Expand AABB to include another AABB.
     */
    void expand_to_include(const AABB& other) {
        min_x = std::min(min_x, other.min_x);
        min_y = std::min(min_y, other.min_y);
        min_z = std::min(min_z, other.min_z);
        max_x = std::max(max_x, other.max_x);
        max_y = std::max(max_y, other.max_y);
        max_z = std::max(max_z, other.max_z);
    }

    /**
     * Merge two AABBs and return the result.
     */
    static AABB merge(const AABB& a, const AABB& b) {
        return AABB(
            std::min(a.min_x, b.min_x),
            std::min(a.min_y, b.min_y),
            std::min(a.min_z, b.min_z),
            std::max(a.max_x, b.max_x),
            std::max(a.max_y, b.max_y),
            std::max(a.max_z, b.max_z)
        );
    }

    /**
     * Check if AABB is valid (min <= max).
     */
    bool is_valid() const {
        return min_x <= max_x && min_y <= max_y && min_z <= max_z;
    }
};

} // namespace spatial
} // namespace astraeus

#endif // ASTRAEUS_SPATIAL_AABB_HPP
