#include <iostream>
#include "core/util/FastMath.hpp"

/**
 * Example demonstrating proper use of Fast Math utilities
 */

using namespace astraeus::math::fast;

void camera_orbit_example() {
    std::cout << "=== Camera Orbit Example ===" << std::endl;
    
    // Orbit camera around origin
    float angle_horizontal = 0.5f;  // radians
    float angle_vertical = 0.3f;    // radians
    float radius = 10.0f;
    
    // Use fast trig for real-time camera updates
    float sin_h, cos_h;
    fastSinCos(angle_horizontal, sin_h, cos_h);
    
    float sin_v, cos_v;
    fastSinCos(angle_vertical, sin_v, cos_v);
    
    // Compute camera position
    float cam_x = radius * cos_v * cos_h;
    float cam_y = radius * sin_v;
    float cam_z = radius * cos_v * sin_h;
    
    std::cout << "Camera position: (" << cam_x << ", " << cam_y << ", " << cam_z << ")" << std::endl;
    
    // Normalize look direction
    float look_x = -cam_x;
    float look_y = -cam_y;
    float look_z = -cam_z;
    fastNormalize(look_x, look_y, look_z);
    
    std::cout << "Look direction: (" << look_x << ", " << look_y << ", " << look_z << ")" << std::endl;
}

void distance_culling_example() {
    std::cout << "\n=== Distance Culling Example ===" << std::endl;
    
    // Entity positions
    float entity_x = 25.0f;
    float entity_y = 30.0f;
    float entity_z = 15.0f;
    
    float camera_x = 0.0f;
    float camera_y = 0.0f;
    float camera_z = 0.0f;
    
    float far_plane = 100.0f;
    
    // Fast distance check for culling
    float dx = entity_x - camera_x;
    float dy = entity_y - camera_y;
    float dz = entity_z - camera_z;
    
    float distance = fastLength(dx, dy, dz);
    
    if (distance > far_plane) {
        std::cout << "Entity culled (distance: " << distance << " > " << far_plane << ")" << std::endl;
    } else {
        std::cout << "Entity visible (distance: " << distance << ")" << std::endl;
    }
}

void lod_selection_example() {
    std::cout << "\n=== LOD Selection Example ===" << std::endl;
    
    float entity_x = 50.0f;
    float entity_y = 0.0f;
    float entity_z = 0.0f;
    
    float camera_x = 0.0f;
    float camera_y = 0.0f;
    float camera_z = 0.0f;
    
    // Fast distance calculation for LOD
    float dx = entity_x - camera_x;
    float dy = entity_y - camera_y;
    float dz = entity_z - camera_z;
    
    float distance = fastLength(dx, dy, dz);
    
    // LOD thresholds
    float lod0_distance = 20.0f;
    float lod1_distance = 50.0f;
    float lod2_distance = 100.0f;
    
    int lod_level;
    if (distance < lod0_distance) {
        lod_level = 0;  // High detail
    } else if (distance < lod1_distance) {
        lod_level = 1;  // Medium detail
    } else if (distance < lod2_distance) {
        lod_level = 2;  // Low detail
    } else {
        lod_level = 3;  // Billboard/impostor
    }
    
    std::cout << "Distance: " << distance << " -> LOD level: " << lod_level << std::endl;
}

void angle_calculation_example() {
    std::cout << "\n=== Angle Calculation Example ===" << std::endl;
    
    // Calculate angle to target for camera look-at
    float target_x = 10.0f;
    float target_z = 10.0f;
    
    float pos_x = 0.0f;
    float pos_z = 0.0f;
    
    float angle = fastAtan2(target_z - pos_z, target_x - pos_x);
    
    std::cout << "Angle to target: " << angle << " radians (" 
              << (angle * RAD_TO_DEG) << " degrees)" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Fast Math Usage Examples" << std::endl;
    std::cout << "  Quality Level: " << ASTRAEUS_FASTMATH_LEVEL << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    camera_orbit_example();
    distance_culling_example();
    lod_selection_example();
    angle_calculation_example();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "All examples completed!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
