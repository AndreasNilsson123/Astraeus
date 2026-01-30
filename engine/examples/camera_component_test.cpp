#include <iostream>
#include <cmath>
#include "scene/World.hpp"

using namespace astraeus;

// Helper to compare floats
bool float_equal(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) < epsilon;
}

int main() {
    std::cout << "=== Camera Component Integration Test ===" << std::endl;
    
    int test_count = 0;
    int passed_count = 0;
    
    World world;
    if (!world.initialize()) {
        std::cerr << "FAILED: Could not initialize world" << std::endl;
        return 1;
    }
    
    // Test 1: Create entity with camera component
    std::cout << "\n=== Test 1: Create Entity with Camera Component ===" << std::endl;
    test_count++;
    {
        uint32_t camera_entity = world.create_entity();
        world.set_entity_camera(camera_entity, CameraProjectionType::Perspective);
        
        CameraComponent* camera = world.get_entity_camera(camera_entity);
        bool success = (camera != nullptr) && 
                      (camera->projection_type == CameraProjectionType::Perspective);
        
        if (success) {
            std::cout << "PASSED: Camera component created on entity" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Camera component not created properly" << std::endl;
        }
    }
    
    // Test 2: Set and get active camera
    std::cout << "\n=== Test 2: Set and Get Active Camera ===" << std::endl;
    test_count++;
    {
        uint32_t camera_entity1 = world.create_entity();
        uint32_t camera_entity2 = world.create_entity();
        
        world.set_entity_camera(camera_entity1, CameraProjectionType::Perspective);
        world.set_entity_camera(camera_entity2, CameraProjectionType::Orthographic);
        
        // Set first camera as active
        world.set_active_camera(camera_entity1);
        
        bool success = (world.get_active_camera_entity() == camera_entity1);
        
        CameraComponent* active_cam = world.get_active_camera();
        success &= (active_cam != nullptr);
        success &= (active_cam->is_active == true);
        success &= (active_cam->projection_type == CameraProjectionType::Perspective);
        
        // Switch to second camera
        world.set_active_camera(camera_entity2);
        success &= (world.get_active_camera_entity() == camera_entity2);
        
        active_cam = world.get_active_camera();
        success &= (active_cam != nullptr);
        success &= (active_cam->projection_type == CameraProjectionType::Orthographic);
        
        // Check that first camera is no longer active
        CameraComponent* cam1 = world.get_entity_camera(camera_entity1);
        success &= (cam1->is_active == false);
        
        if (success) {
            std::cout << "PASSED: Active camera management works correctly" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Active camera management incorrect" << std::endl;
        }
    }
    
    // Test 3: Update camera from entity transform
    std::cout << "\n=== Test 3: Update Camera from Entity Transform ===" << std::endl;
    test_count++;
    {
        uint32_t camera_entity = world.create_entity();
        world.set_entity_camera(camera_entity, CameraProjectionType::Perspective);
        
        // Set entity position
        world.set_entity_transform(camera_entity,
                                  5.0f, 3.0f, 7.0f,  // position
                                  0.0f, 0.0f, 0.0f,  // rotation
                                  1.0f, 1.0f, 1.0f); // scale
        
        // Update transforms
        world.update_world_transforms();
        
        // Update camera
        float aspect_ratio = 16.0f / 9.0f;
        world.update_entity_camera(camera_entity, aspect_ratio);
        
        CameraComponent* camera = world.get_entity_camera(camera_entity);
        
        // Check that view matrix is not identity
        bool is_identity = true;
        for (int i = 0; i < 16; ++i) {
            float expected = (i == 0 || i == 5 || i == 10 || i == 15) ? 1.0f : 0.0f;
            if (!float_equal(camera->view_matrix[i], expected)) {
                is_identity = false;
                break;
            }
        }
        
        bool success = !is_identity && !camera->view_dirty && !camera->projection_dirty;
        
        if (success) {
            std::cout << "PASSED: Camera updated from entity transform" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Camera update from transform incorrect" << std::endl;
        }
    }
    
    // Test 4: Build camera uniforms
    std::cout << "\n=== Test 4: Build Camera Uniforms ===" << std::endl;
    test_count++;
    {
        uint32_t camera_entity = world.create_entity();
        world.set_entity_camera(camera_entity, CameraProjectionType::Perspective);
        
        // Set entity position
        world.set_entity_transform(camera_entity,
                                  10.0f, 5.0f, 8.0f,  // position
                                  0.0f, 0.0f, 0.0f,   // rotation
                                  1.0f, 1.0f, 1.0f);  // scale
        
        // Update transforms and camera
        world.update_world_transforms();
        float aspect_ratio = 16.0f / 9.0f;
        world.update_entity_camera(camera_entity, aspect_ratio);
        
        // Build uniforms
        CameraUniforms uniforms;
        world.build_camera_uniforms(camera_entity, uniforms);
        
        // Verify position
        bool success = float_equal(uniforms.camera_position[0], 10.0f) &&
                      float_equal(uniforms.camera_position[1], 5.0f) &&
                      float_equal(uniforms.camera_position[2], 8.0f);
        
        if (success) {
            std::cout << "PASSED: Camera uniforms built correctly" << std::endl;
            std::cout << "  Position: (" << uniforms.camera_position[0] << ", "
                     << uniforms.camera_position[1] << ", "
                     << uniforms.camera_position[2] << ")" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Camera uniforms incorrect" << std::endl;
        }
    }
    
    // Test 5: Remove camera component
    std::cout << "\n=== Test 5: Remove Camera Component ===" << std::endl;
    test_count++;
    {
        uint32_t camera_entity = world.create_entity();
        world.set_entity_camera(camera_entity, CameraProjectionType::Perspective);
        world.set_active_camera(camera_entity);
        
        // Verify camera exists
        bool had_camera = (world.get_entity_camera(camera_entity) != nullptr);
        bool was_active = (world.get_active_camera_entity() == camera_entity);
        
        // Remove camera
        world.remove_entity_camera(camera_entity);
        
        bool success = had_camera && was_active &&
                      (world.get_entity_camera(camera_entity) == nullptr) &&
                      (world.get_active_camera_entity() == 0);
        
        if (success) {
            std::cout << "PASSED: Camera component removed and active camera cleared" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Camera component removal incorrect" << std::endl;
        }
    }
    
    // Test 6: Destroy entity with camera component
    std::cout << "\n=== Test 6: Destroy Entity with Camera Component ===" << std::endl;
    test_count++;
    {
        uint32_t camera_entity = world.create_entity();
        world.set_entity_camera(camera_entity, CameraProjectionType::Perspective);
        world.set_active_camera(camera_entity);
        
        // Destroy entity
        world.destroy_entity(camera_entity);
        
        bool success = (world.get_entity_camera(camera_entity) == nullptr) &&
                      (world.get_active_camera_entity() == 0) &&
                      (world.get_active_camera() == nullptr);
        
        if (success) {
            std::cout << "PASSED: Entity with camera destroyed cleanly" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Entity destruction with camera incorrect" << std::endl;
        }
    }
    
    // Test 7: Multiple cameras and update all
    std::cout << "\n=== Test 7: Multiple Cameras and Update All ===" << std::endl;
    test_count++;
    {
        uint32_t cam1 = world.create_entity();
        uint32_t cam2 = world.create_entity();
        uint32_t cam3 = world.create_entity();
        
        world.set_entity_camera(cam1, CameraProjectionType::Perspective);
        world.set_entity_camera(cam2, CameraProjectionType::Orthographic);
        world.set_entity_camera(cam3, CameraProjectionType::Perspective);
        
        // Set different positions
        world.set_entity_transform(cam1, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        world.set_entity_transform(cam2, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        world.set_entity_transform(cam3, 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        
        world.update_world_transforms();
        
        // Update all cameras at once
        float aspect_ratio = 1.0f;
        world.update_all_cameras(aspect_ratio);
        
        CameraComponent* camera1 = world.get_entity_camera(cam1);
        CameraComponent* camera2 = world.get_entity_camera(cam2);
        CameraComponent* camera3 = world.get_entity_camera(cam3);
        
        bool success = (camera1 != nullptr && !camera1->view_dirty && !camera1->projection_dirty) &&
                      (camera2 != nullptr && !camera2->view_dirty && !camera2->projection_dirty) &&
                      (camera3 != nullptr && !camera3->view_dirty && !camera3->projection_dirty);
        
        if (success) {
            std::cout << "PASSED: Multiple cameras updated successfully" << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Multiple camera update incorrect" << std::endl;
        }
    }
    
    // Test 8: Frustum culling with camera component
    std::cout << "\n=== Test 8: Frustum Culling with Camera Component ===" << std::endl;
    test_count++;
    {
        uint32_t camera_entity = world.create_entity();
        world.set_entity_camera(camera_entity, CameraProjectionType::Perspective);
        
        // Position camera looking at origin
        world.set_entity_transform(camera_entity,
                                  0.0f, 0.0f, 10.0f,  // position
                                  0.0f, 0.0f, 0.0f,   // rotation
                                  1.0f, 1.0f, 1.0f);  // scale
        
        world.update_world_transforms();
        world.update_entity_camera(camera_entity, 1.0f);
        
        CameraComponent* camera = world.get_entity_camera(camera_entity);
        
        // Test frustum contains origin
        bool contains_origin = camera->frustum.contains_point(0.0f, 0.0f, 0.0f);
        
        // Test frustum doesn't contain point far behind camera
        bool contains_behind = camera->frustum.contains_point(0.0f, 0.0f, 50.0f);
        
        bool success = contains_origin && !contains_behind;
        
        if (success) {
            std::cout << "PASSED: Frustum culling works with camera component" << std::endl;
            std::cout << "  Origin visible: " << (contains_origin ? "YES" : "NO") << std::endl;
            std::cout << "  Point behind camera: " << (contains_behind ? "VISIBLE" : "CULLED") << std::endl;
            passed_count++;
        } else {
            std::cout << "FAILED: Frustum culling incorrect" << std::endl;
        }
    }
    
    // Summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Tests passed: " << passed_count << "/" << test_count << std::endl;
    
    world.shutdown();
    
    if (passed_count == test_count) {
        std::cout << "All tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests FAILED!" << std::endl;
        return 1;
    }
}
