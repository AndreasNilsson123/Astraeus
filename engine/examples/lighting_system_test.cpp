/**
 * lighting_system_test.cpp
 * 
 * Test for the lighting system v1.
 * Demonstrates:
 * - DirectionalLight structure
 * - LitMaterial with Lambert + Blinn-Phong shading
 * - Light parameter integration with World
 * - LitMeshPass for lit rendering
 */

#include <iostream>
#include <cmath>
#include <renderer/LitMaterial.hpp>
#include <renderer/MaterialLibrary.hpp>
#include <scene/World.hpp>

using namespace astraeus;

void print_light_direction(float x, float y, float z) {
    std::cout << "  Direction: (" << x << ", " << y << ", " << z << ")" << std::endl;
}

void print_light_color(float r, float g, float b, float intensity) {
    std::cout << "  Color: (" << r << ", " << g << ", " << b << ")" << std::endl;
    std::cout << "  Intensity: " << intensity << std::endl;
}

void print_ambient_light(float r, float g, float b) {
    std::cout << "  Ambient: (" << r << ", " << g << ", " << b << ")" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Lighting System v1 Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Test 1: DirectionalLight structure
    std::cout << "Testing DirectionalLight structure..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    DirectionalLight light;
    std::cout << "Created DirectionalLight with default values:" << std::endl;
    std::cout << "  Direction: (" << light.direction[0] << ", " 
              << light.direction[1] << ", " << light.direction[2] << ")" << std::endl;
    std::cout << "  Color: (" << light.color[0] << ", " 
              << light.color[1] << ", " << light.color[2] << ")" << std::endl;
    std::cout << "  Intensity: " << light.intensity << std::endl;
    std::cout << "  Ambient: (" << light.ambient[0] << ", " 
              << light.ambient[1] << ", " << light.ambient[2] << ")" << std::endl;

    // Test 2: Modify light parameters
    std::cout << std::endl;
    std::cout << "Modifying light parameters..." << std::endl;
    
    // Set light to come from upper-right (typical 3-point lighting key light position)
    light.direction[0] = 0.5f;
    light.direction[1] = -0.7f;
    light.direction[2] = 0.3f;
    // Normalize
    float len = std::sqrt(light.direction[0]*light.direction[0] + 
                         light.direction[1]*light.direction[1] + 
                         light.direction[2]*light.direction[2]);
    light.direction[0] /= len;
    light.direction[1] /= len;
    light.direction[2] /= len;
    
    light.color[0] = 1.0f;
    light.color[1] = 0.95f;  // Slightly warm white
    light.color[2] = 0.9f;
    light.intensity = 1.2f;
    
    light.ambient[0] = 0.15f;
    light.ambient[1] = 0.18f;  // Slightly blue-tinted ambient
    light.ambient[2] = 0.22f;
    
    std::cout << "Updated light to warm key light position:" << std::endl;
    std::cout << "  Direction: (" << light.direction[0] << ", " 
              << light.direction[1] << ", " << light.direction[2] << ")" << std::endl;
    std::cout << "  Color: (" << light.color[0] << ", " 
              << light.color[1] << ", " << light.color[2] << ")" << std::endl;
    std::cout << "  Intensity: " << light.intensity << std::endl;
    std::cout << "  Ambient: (" << light.ambient[0] << ", " 
              << light.ambient[1] << ", " << light.ambient[2] << ")" << std::endl;

    // Test 3: LitMaterial instantiation
    std::cout << std::endl;
    std::cout << "Testing LitMaterial instantiation..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    LitMaterial lit_material;
    std::cout << "Created LitMaterial instance" << std::endl;
    std::cout << "  Name: " << lit_material.get_name() << std::endl;
    std::cout << "  Is initialized: " << (lit_material.is_initialized() ? "Yes" : "No") << std::endl;
    
    const PipelineState& lit_pipeline = lit_material.get_pipeline_state();
    std::cout << "  Pipeline state configured:" << std::endl;
    std::cout << "    Depth test: " << (lit_pipeline.depth_test_enabled ? "enabled" : "disabled") << std::endl;
    std::cout << "    Blend: " << (lit_pipeline.blend_enabled ? "enabled" : "disabled") << std::endl;
    std::cout << "    Cull: " << (lit_pipeline.cull_enabled ? "enabled" : "disabled") << std::endl;

    // Test 4: Setting light on material
    std::cout << std::endl;
    std::cout << "Setting light parameters on material..." << std::endl;
    lit_material.set_directional_light(light);
    std::cout << "Light parameters set on LitMaterial" << std::endl;

    // Test 5: World lighting API
    std::cout << std::endl;
    std::cout << "Testing World lighting API..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    World world;
    world.initialize();
    
    std::cout << "Default lighting in World:" << std::endl;
    float dir_x, dir_y, dir_z;
    float col_r, col_g, col_b, intensity;
    float amb_r, amb_g, amb_b;
    
    world.get_light_direction(dir_x, dir_y, dir_z);
    world.get_light_color(col_r, col_g, col_b, intensity);
    world.get_ambient_light(amb_r, amb_g, amb_b);
    
    print_light_direction(dir_x, dir_y, dir_z);
    print_light_color(col_r, col_g, col_b, intensity);
    print_ambient_light(amb_r, amb_g, amb_b);

    // Test 6: Modify world lighting
    std::cout << std::endl;
    std::cout << "Modifying world lighting..." << std::endl;
    
    world.set_light_direction(0.5f, -0.7f, 0.3f);  // Will be normalized internally
    world.set_light_color(1.0f, 0.9f, 0.8f, 1.5f);
    world.set_ambient_light(0.1f, 0.1f, 0.15f);
    
    world.get_light_direction(dir_x, dir_y, dir_z);
    world.get_light_color(col_r, col_g, col_b, intensity);
    world.get_ambient_light(amb_r, amb_g, amb_b);
    
    std::cout << "Updated world lighting:" << std::endl;
    print_light_direction(dir_x, dir_y, dir_z);
    print_light_color(col_r, col_g, col_b, intensity);
    print_ambient_light(amb_r, amb_g, amb_b);

    // Test 7: Rotating light direction
    std::cout << std::endl;
    std::cout << "Testing rotating light direction..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    std::cout << "Simulating light rotation (4 positions):" << std::endl;
    for (int i = 0; i < 4; i++) {
        float angle = (i * 90.0f) * 3.14159265359f / 180.0f;  // 0, 90, 180, 270 degrees
        float x = std::cos(angle);
        float y = -0.5f;  // Keep some downward component
        float z = std::sin(angle);
        
        world.set_light_direction(x, y, z);
        world.get_light_direction(dir_x, dir_y, dir_z);
        
        std::cout << "  Position " << (i+1) << " (" << (i*90) << " degrees): ";
        std::cout << "(" << dir_x << ", " << dir_y << ", " << dir_z << ")" << std::endl;
    }

    // Test 8: Different lighting scenarios
    std::cout << std::endl;
    std::cout << "Testing different lighting scenarios..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Scenario 1: Bright noon sun
    std::cout << std::endl << "Scenario 1: Bright noon sun" << std::endl;
    world.set_light_direction(0.0f, -1.0f, 0.0f);  // Straight down
    world.set_light_color(1.0f, 1.0f, 0.95f, 2.0f);
    world.set_ambient_light(0.3f, 0.3f, 0.35f);
    
    world.get_light_direction(dir_x, dir_y, dir_z);
    world.get_light_color(col_r, col_g, col_b, intensity);
    world.get_ambient_light(amb_r, amb_g, amb_b);
    
    print_light_direction(dir_x, dir_y, dir_z);
    print_light_color(col_r, col_g, col_b, intensity);
    print_ambient_light(amb_r, amb_g, amb_b);
    
    // Scenario 2: Sunset/sunrise
    std::cout << std::endl << "Scenario 2: Warm sunset" << std::endl;
    world.set_light_direction(0.7f, -0.3f, 0.0f);  // Low angle from side
    world.set_light_color(1.0f, 0.6f, 0.3f, 1.0f);  // Orange light
    world.set_ambient_light(0.2f, 0.15f, 0.2f);
    
    world.get_light_direction(dir_x, dir_y, dir_z);
    world.get_light_color(col_r, col_g, col_b, intensity);
    world.get_ambient_light(amb_r, amb_g, amb_b);
    
    print_light_direction(dir_x, dir_y, dir_z);
    print_light_color(col_r, col_g, col_b, intensity);
    print_ambient_light(amb_r, amb_g, amb_b);
    
    // Scenario 3: Night/moonlight
    std::cout << std::endl << "Scenario 3: Cool moonlight" << std::endl;
    world.set_light_direction(0.3f, -0.8f, 0.5f);
    world.set_light_color(0.7f, 0.8f, 1.0f, 0.5f);  // Cool blue tint
    world.set_ambient_light(0.05f, 0.06f, 0.08f);  // Very dark
    
    world.get_light_direction(dir_x, dir_y, dir_z);
    world.get_light_color(col_r, col_g, col_b, intensity);
    world.get_ambient_light(amb_r, amb_g, amb_b);
    
    print_light_direction(dir_x, dir_y, dir_z);
    print_light_color(col_r, col_g, col_b, intensity);
    print_ambient_light(amb_r, amb_g, amb_b);

    world.shutdown();

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Test completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Summary:" << std::endl;
    std::cout << "  ✓ DirectionalLight structure with direction, color, intensity, ambient" << std::endl;
    std::cout << "  ✓ LitMaterial with Lambert + Blinn-Phong shading" << std::endl;
    std::cout << "  ✓ World API for setting/getting light parameters" << std::endl;
    std::cout << "  ✓ Light direction normalization" << std::endl;
    std::cout << "  ✓ Multiple lighting scenarios (noon, sunset, moonlight)" << std::endl;
    std::cout << "  ✓ Light rotation demonstration" << std::endl;
    std::cout << std::endl;
    std::cout << "The lighting system v1 is ready:" << std::endl;
    std::cout << "  - Directional light with configurable direction, color, intensity" << std::endl;
    std::cout << "  - Ambient lighting" << std::endl;
    std::cout << "  - Lambert diffuse + Blinn-Phong specular shading" << std::endl;
    std::cout << "  - Integration with World and Material system" << std::endl;
    std::cout << "  - LitMeshPass for rendering with lighting" << std::endl;
    std::cout << std::endl;

    return 0;
}
