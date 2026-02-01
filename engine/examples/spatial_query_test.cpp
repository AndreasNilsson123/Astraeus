#include <iostream>
#include <cmath>
#include <vector>
#include "scene/World.hpp"
#include "scene/spatial/SpatialIndex.hpp"

using namespace astraeus;
using namespace astraeus::spatial;

// Helper to compare floats
bool float_equal(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) < epsilon;
}

void print_test_result(const char* test_name, bool passed) {
    if (passed) {
        std::cout << "  ✓ " << test_name << " PASSED" << std::endl;
    } else {
        std::cout << "  ✗ " << test_name << " FAILED" << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Spatial Query System Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: AABB basic operations
    {
        std::cout << "\n=== Test 1: AABB Operations ===" << std::endl;
        
        AABB box1(0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f);
        AABB box2(1.0f, 1.0f, 1.0f, 3.0f, 3.0f, 3.0f);
        AABB box3(10.0f, 10.0f, 10.0f, 12.0f, 12.0f, 12.0f);
        
        // Test intersection
        bool test1a = box1.intersects(box2);
        bool test1b = !box1.intersects(box3);
        print_test_result("AABB intersection (overlapping)", test1a);
        print_test_result("AABB intersection (non-overlapping)", test1b);
        total_tests += 2;
        if (test1a) passed_tests++;
        if (test1b) passed_tests++;
        
        // Test point containment
        bool test1c = box1.contains_point(1.0f, 1.0f, 1.0f);
        bool test1d = !box1.contains_point(5.0f, 5.0f, 5.0f);
        print_test_result("AABB contains point (inside)", test1c);
        print_test_result("AABB contains point (outside)", test1d);
        total_tests += 2;
        if (test1c) passed_tests++;
        if (test1d) passed_tests++;
        
        // Test merge
        AABB merged = AABB::merge(box1, box2);
        bool test1e = float_equal(merged.min_x, 0.0f) && 
                      float_equal(merged.max_x, 3.0f);
        print_test_result("AABB merge", test1e);
        total_tests++;
        if (test1e) passed_tests++;
    }
    
    // Test 2: Ray-AABB intersection
    {
        std::cout << "\n=== Test 2: Ray-AABB Intersection ===" << std::endl;
        
        Ray ray(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        ray.normalize();
        
        AABB box(2.0f, -1.0f, -1.0f, 4.0f, 1.0f, 1.0f);
        
        float t;
        bool test2a = ray.intersects_aabb(box, 0.0f, 100.0f, t);
        bool test2b = float_equal(t, 2.0f);
        print_test_result("Ray hits AABB", test2a);
        print_test_result("Ray hit distance correct", test2b);
        total_tests += 2;
        if (test2a) passed_tests++;
        if (test2b) passed_tests++;
        
        // Test ray miss
        Ray miss_ray(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        miss_ray.normalize();
        bool test2c = !miss_ray.intersects_aabb(box, 0.0f, 100.0f, t);
        print_test_result("Ray misses AABB", test2c);
        total_tests++;
        if (test2c) passed_tests++;
    }
    
    // Test 3: BVH construction and queries
    {
        std::cout << "\n=== Test 3: BVH Construction ===" << std::endl;
        
        BVH bvh;
        std::vector<BVH::Entry> entries;
        
        // Add some test entities
        entries.emplace_back(1, AABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
        entries.emplace_back(2, AABB(5.0f, -1.0f, -1.0f, 7.0f, 1.0f, 1.0f));
        entries.emplace_back(3, AABB(10.0f, -1.0f, -1.0f, 12.0f, 1.0f, 1.0f));
        
        bvh.build(entries);
        
        bool test3a = bvh.is_built();
        print_test_result("BVH builds successfully", test3a);
        total_tests++;
        if (test3a) passed_tests++;
        
        // Test raycast
        Ray ray(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        ray.normalize();
        
        std::vector<RayHit> hits;
        bvh.raycast(ray, 100.0f, hits);
        
        bool test3b = (hits.size() == 3);
        bool test3c = (hits.size() > 0 && hits[0].entity_id == 1); // Closest entity
        print_test_result("BVH raycast finds all entities", test3b);
        print_test_result("BVH raycast sorts by distance", test3c);
        total_tests += 2;
        if (test3b) passed_tests++;
        if (test3c) passed_tests++;
        
        // Test nearest
        uint32_t nearest_id;
        float nearest_dist;
        bool found = bvh.nearest(0.5f, 0.0f, 0.0f, 100.0f, nearest_id, nearest_dist);
        bool test3d = found && (nearest_id == 1);
        print_test_result("BVH nearest query", test3d);
        total_tests++;
        if (test3d) passed_tests++;
        
        // Test AABB query
        std::vector<uint32_t> aabb_results;
        AABB query_box(-2.0f, -2.0f, -2.0f, 2.0f, 2.0f, 2.0f);
        bvh.query_aabb(query_box, aabb_results);
        bool test3e = (aabb_results.size() == 1 && aabb_results[0] == 1);
        print_test_result("BVH AABB query", test3e);
        total_tests++;
        if (test3e) passed_tests++;
    }
    
    // Test 4: World integration
    {
        std::cout << "\n=== Test 4: World Integration ===" << std::endl;
        
        World world;
        world.initialize();
        
        // Create entities at different positions
        uint32_t e1 = world.create_entity();
        world.set_entity_transform(e1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        
        uint32_t e2 = world.create_entity();
        world.set_entity_transform(e2, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        
        uint32_t e3 = world.create_entity();
        world.set_entity_transform(e3, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        
        // Rebuild spatial index
        world.rebuild_spatial_index();
        
        // Test raycast through world
        std::vector<RayHit> hits;
        bool found = world.raycast(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 100.0f, hits);
        bool test4a = found && (hits.size() == 3);
        bool test4b = (hits.size() > 0 && hits[0].entity_id == e1);
        print_test_result("World raycast integration", test4a);
        print_test_result("World raycast returns closest first", test4b);
        total_tests += 2;
        if (test4a) passed_tests++;
        if (test4b) passed_tests++;
        
        // Test nearest query
        uint32_t nearest_id;
        float nearest_dist;
        bool nearest_found = world.nearest_entity(0.5f, 0.0f, 0.0f, 100.0f, nearest_id, nearest_dist);
        bool test4c = nearest_found && (nearest_id == e1);
        print_test_result("World nearest query integration", test4c);
        total_tests++;
        if (test4c) passed_tests++;
        
        world.shutdown();
    }
    
    // Test 5: Performance/Scale test
    {
        std::cout << "\n=== Test 5: Performance (100 entities) ===" << std::endl;
        
        World world;
        world.initialize();
        
        // Create 100 entities in a grid
        const int grid_size = 10;
        for (int i = 0; i < grid_size; ++i) {
            for (int j = 0; j < grid_size; ++j) {
                uint32_t e = world.create_entity();
                world.set_entity_transform(e, 
                    static_cast<float>(i * 5), 
                    static_cast<float>(j * 5), 
                    0.0f, 
                    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
            }
        }
        
        world.rebuild_spatial_index();
        
        // Raycast through the grid
        std::vector<RayHit> hits;
        bool found = world.raycast(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1000.0f, hits);
        
        bool test5a = found && (hits.size() >= 10); // Should hit diagonal line
        bool test5b = true;
        // Verify sorting (each hit should be closer or equal than the next)
        for (size_t i = 1; i < hits.size(); ++i) {
            if (hits[i].distance < hits[i-1].distance) {
                test5b = false;
                break;
            }
        }
        
        print_test_result("Raycast through 100 entities", test5a);
        print_test_result("Results properly sorted", test5b);
        total_tests += 2;
        if (test5a) passed_tests++;
        if (test5b) passed_tests++;
        
        world.shutdown();
    }
    
    // Summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Test Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Passed: " << passed_tests << "/" << total_tests << std::endl;
    
    if (passed_tests == total_tests) {
        std::cout << "✓ ALL TESTS PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "✗ SOME TESTS FAILED" << std::endl;
        return 1;
    }
}
