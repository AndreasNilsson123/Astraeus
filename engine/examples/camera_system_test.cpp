#include <iostream>
#include <cmath>
#include "scene/CameraComponent.hpp"
#include "scene/CameraSystem.hpp"

using namespace astraeus;

// Helper to compare floats
bool float_equal(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) < epsilon;
}

// Helper to print matrix
void print_matrix(const char* name, const float* m) {
    std::cout << name << ":\n";
    for (int row = 0; row < 4; ++row) {
        std::cout << "  ";
        for (int col = 0; col < 4; ++col) {
            std::cout << m[col * 4 + row] << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    std::cout << "=== Camera System Test ===" << std::endl;
    
    int test_count = 0;
    int passed_count = 0;
    
    // Test 1: CameraComponent initialization
    std::cout << "\n=== Test 1: CameraComponent Initialization ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        
        bool success = true;
        success &= camera.projection_type == CameraProjectionType::Perspective;
        success &= float_equal(camera.fov_degrees, 60.0f);
        success &= float_equal(camera.near_plane, 0.1f);
        success &= float_equal(camera.far_plane, 1000.0f);
        success &= float_equal(camera.exposure, 1.0f);
        success &= camera.view_dirty == true;
        success &= camera.projection_dirty == true;
        success &= camera.is_active == false;
        
        if (success) {
            std::cout << "PASSED: Camera initialized with correct defaults" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Camera initialization incorrect" << std::endl;
        }
    }
    
    // Test 2: Perspective projection setup
    std::cout << "\n=== Test 2: Perspective Projection Setup ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        CameraSystem::set_perspective(camera, 45.0f, 0.5f, 500.0f);
        
        bool success = true;
        success &= camera.projection_type == CameraProjectionType::Perspective;
        success &= float_equal(camera.fov_degrees, 45.0f);
        success &= float_equal(camera.near_plane, 0.5f);
        success &= float_equal(camera.far_plane, 500.0f);
        success &= camera.projection_dirty == true;
        
        if (success) {
            std::cout << "PASSED: Perspective projection parameters set correctly" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Perspective projection setup incorrect" << std::endl;
        }
    }
    
    // Test 3: Orthographic projection setup
    std::cout << "\n=== Test 3: Orthographic Projection Setup ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        CameraSystem::set_orthographic(camera, -5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
        
        bool success = true;
        success &= camera.projection_type == CameraProjectionType::Orthographic;
        success &= float_equal(camera.ortho_left, -5.0f);
        success &= float_equal(camera.ortho_right, 5.0f);
        success &= float_equal(camera.ortho_bottom, -5.0f);
        success &= float_equal(camera.ortho_top, 5.0f);
        success &= float_equal(camera.near_plane, 0.1f);
        success &= float_equal(camera.far_plane, 100.0f);
        success &= camera.projection_dirty == true;
        
        if (success) {
            std::cout << "PASSED: Orthographic projection parameters set correctly" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Orthographic projection setup incorrect" << std::endl;
        }
    }
    
    // Test 4: View matrix update
    std::cout << "\n=== Test 4: View Matrix Update ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        
        // Position camera at (10, 5, 10) looking at origin
        float eye_x = 10.0f, eye_y = 5.0f, eye_z = 10.0f;
        float target_x = 0.0f, target_y = 0.0f, target_z = 0.0f;
        float up_x = 0.0f, up_y = 1.0f, up_z = 0.0f;
        
        CameraSystem::update_view_matrix(camera, eye_x, eye_y, eye_z, target_x, target_y, target_z, up_x, up_y, up_z);
        
        // View matrix should be non-identity
        bool is_identity = true;
        for (int i = 0; i < 16; ++i) {
            float expected = (i == 0 || i == 5 || i == 10 || i == 15) ? 1.0f : 0.0f;
            if (!float_equal(camera.view_matrix[i], expected)) {
                is_identity = false;
                break;
            }
        }
        
        bool success = !is_identity && !camera.view_dirty;
        
        if (success) {
            std::cout << "PASSED: View matrix updated correctly" << std::endl;
            print_matrix("View Matrix", camera.view_matrix);
            passed_count++;
        } else {
            std::cout << "FAILED: View matrix update incorrect" << std::endl;
        }
    }
    
    // Test 5: Projection matrix update (perspective)
    std::cout << "\n=== Test 5: Projection Matrix Update (Perspective) ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        CameraSystem::set_perspective(camera, 60.0f, 0.1f, 1000.0f);
        
        float aspect_ratio = 16.0f / 9.0f;
        CameraSystem::update_projection_matrix(camera, aspect_ratio);
        
        // Projection matrix should be non-identity
        bool is_identity = true;
        for (int i = 0; i < 16; ++i) {
            float expected = (i == 0 || i == 5 || i == 10 || i == 15) ? 1.0f : 0.0f;
            if (!float_equal(camera.projection_matrix[i], expected)) {
                is_identity = false;
                break;
            }
        }
        
        bool success = !is_identity && !camera.projection_dirty;
        
        if (success) {
            std::cout << "PASSED: Perspective projection matrix updated correctly" << std::endl;
            print_matrix("Projection Matrix", camera.projection_matrix);
            passed_count++;
        } else {
            std::cout << "FAILED: Perspective projection matrix update incorrect" << std::endl;
        }
    }
    
    // Test 6: Projection matrix update (orthographic)
    std::cout << "\n=== Test 6: Projection Matrix Update (Orthographic) ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        CameraSystem::set_orthographic(camera, -10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
        
        float aspect_ratio = 1.0f;
        CameraSystem::update_projection_matrix(camera, aspect_ratio);
        
        // Check that matrix values are reasonable (non-zero diagonal)
        bool success = camera.projection_matrix[0] > 0.0f &&
                      camera.projection_matrix[5] > 0.0f &&
                      camera.projection_matrix[10] < 0.0f && // Should be negative for orthographic
                      float_equal(camera.projection_matrix[15], 1.0f);
        
        if (success) {
            std::cout << "PASSED: Orthographic projection matrix updated correctly" << std::endl;
            print_matrix("Projection Matrix", camera.projection_matrix);
            passed_count++;
        } else {
            std::cout << "FAILED: Orthographic projection matrix update incorrect" << std::endl;
        }
    }
    
    // Test 7: Full camera update with frustum extraction
    std::cout << "\n=== Test 7: Full Camera Update with Frustum ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        CameraSystem::set_perspective(camera, 60.0f, 0.1f, 1000.0f);
        
        float eye_x = 10.0f, eye_y = 5.0f, eye_z = 10.0f;
        float target_x = 0.0f, target_y = 0.0f, target_z = 0.0f;
        float up_x = 0.0f, up_y = 1.0f, up_z = 0.0f;
        float aspect_ratio = 16.0f / 9.0f;
        
        CameraSystem::update_camera(camera, eye_x, eye_y, eye_z, target_x, target_y, target_z, up_x, up_y, up_z, aspect_ratio);
        
        bool success = !camera.view_dirty && !camera.projection_dirty;
        
        // Test frustum contains origin (which is at target)
        bool contains_origin = camera.frustum.contains_point(0.0f, 0.0f, 0.0f);
        success &= contains_origin;
        
        if (success) {
            std::cout << "PASSED: Full camera update with frustum extraction" << std::endl;
            std::cout << "Frustum contains origin: " << (contains_origin ? "YES" : "NO") << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Full camera update incorrect" << std::endl;
        }
    }
    
    // Test 8: Frustum culling - sphere intersection
    std::cout << "\n=== Test 8: Frustum Culling - Sphere Intersection ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        CameraSystem::set_perspective(camera, 60.0f, 0.1f, 100.0f);
        
        float eye_x = 0.0f, eye_y = 0.0f, eye_z = 10.0f;
        float target_x = 0.0f, target_y = 0.0f, target_z = 0.0f;
        float up_x = 0.0f, up_y = 1.0f, up_z = 0.0f;
        float aspect_ratio = 1.0f;
        
        CameraSystem::update_camera(camera, eye_x, eye_y, eye_z, target_x, target_y, target_z, up_x, up_y, up_z, aspect_ratio);
        
        // Test sphere at origin (should be visible)
        bool visible_center = camera.frustum.intersects_sphere(0.0f, 0.0f, 0.0f, 1.0f);
        
        // Test sphere far behind camera (should not be visible)
        bool visible_behind = camera.frustum.intersects_sphere(0.0f, 0.0f, 200.0f, 1.0f);
        
        bool success = visible_center && !visible_behind;
        
        if (success) {
            std::cout << "PASSED: Frustum sphere culling works correctly" << std::endl;
            std::cout << "  Sphere at origin: " << (visible_center ? "VISIBLE" : "CULLED") << std::endl;
            std::cout << "  Sphere behind camera: " << (visible_behind ? "VISIBLE" : "CULLED") << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Frustum sphere culling incorrect" << std::endl;
        }
    }
    
    // Test 9: Camera uniforms pack
    std::cout << "\n=== Test 9: Camera Uniforms Pack ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        CameraSystem::set_perspective(camera, 45.0f, 0.5f, 500.0f);
        
        float eye_x = 5.0f, eye_y = 3.0f, eye_z = 7.0f;
        float target_x = 0.0f, target_y = 0.0f, target_z = 0.0f;
        float up_x = 0.0f, up_y = 1.0f, up_z = 0.0f;
        float aspect_ratio = 16.0f / 9.0f;
        
        CameraSystem::update_camera(camera, eye_x, eye_y, eye_z, target_x, target_y, target_z, up_x, up_y, up_z, aspect_ratio);
        
        CameraUniforms uniforms;
        CameraSystem::build_camera_uniforms(camera, eye_x, eye_y, eye_z, uniforms);
        
        bool success = float_equal(uniforms.camera_position[0], eye_x) &&
                      float_equal(uniforms.camera_position[1], eye_y) &&
                      float_equal(uniforms.camera_position[2], eye_z) &&
                      float_equal(uniforms.near_plane, 0.5f) &&
                      float_equal(uniforms.far_plane, 500.0f) &&
                      float_equal(uniforms.fov_degrees, 45.0f);
        
        if (success) {
            std::cout << "PASSED: Camera uniforms pack created correctly" << std::endl;
            std::cout << "  Position: (" << uniforms.camera_position[0] << ", " 
                     << uniforms.camera_position[1] << ", " 
                     << uniforms.camera_position[2] << ")" << std::endl;
            std::cout << "  Direction: (" << uniforms.camera_direction[0] << ", " 
                     << uniforms.camera_direction[1] << ", " 
                     << uniforms.camera_direction[2] << ")" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Camera uniforms pack incorrect" << std::endl;
        }
    }
    
    // Test 10: AABB frustum culling
    std::cout << "\n=== Test 10: Frustum Culling - AABB Intersection ===" << std::endl;
    test_count++;
    {
        CameraComponent camera;
        CameraSystem::set_perspective(camera, 60.0f, 0.1f, 100.0f);
        
        float eye_x = 0.0f, eye_y = 0.0f, eye_z = 10.0f;
        float target_x = 0.0f, target_y = 0.0f, target_z = 0.0f;
        float up_x = 0.0f, up_y = 1.0f, up_z = 0.0f;
        float aspect_ratio = 1.0f;
        
        CameraSystem::update_camera(camera, eye_x, eye_y, eye_z, target_x, target_y, target_z, up_x, up_y, up_z, aspect_ratio);
        
        // Test AABB at origin (should be visible)
        bool visible_center = camera.frustum.intersects_aabb(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
        
        // Test AABB far to the side (should not be visible)
        bool visible_side = camera.frustum.intersects_aabb(100.0f, 0.0f, 0.0f, 101.0f, 1.0f, 1.0f);
        
        bool success = visible_center && !visible_side;
        
        if (success) {
            std::cout << "PASSED: Frustum AABB culling works correctly" << std::endl;
            std::cout << "  AABB at origin: " << (visible_center ? "VISIBLE" : "CULLED") << std::endl;
            std::cout << "  AABB far to side: " << (visible_side ? "VISIBLE" : "CULLED") << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Frustum AABB culling incorrect" << std::endl;
        }
    }
    
    // Summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Tests passed: " << passed_count << "/" << test_count << std::endl;
    
    if (passed_count == test_count) {
        std::cout << "All tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests FAILED!" << std::endl;
        return 1;
    }
}
