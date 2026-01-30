#include <iostream>
#include <cmath>
#include <cstring>
#include "scene/World.hpp"

using namespace astraeus;

// Helper to compare floats
bool float_equal(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) < epsilon;
}

// Helper to compare matrices
bool matrix_equal(const float* a, const float* b, float epsilon = 0.001f) {
    for (int i = 0; i < 16; ++i) {
        if (!float_equal(a[i], b[i], epsilon)) {
            return false;
        }
    }
    return true;
}

int main() {
    std::cout << "=== Scene Hierarchy & Transform Test ===" << std::endl;
    
    World world;
    if (!world.initialize()) {
        std::cerr << "FAILED: Could not initialize world" << std::endl;
        return 1;
    }
    
    // Test 1: Basic entity creation with names
    std::cout << "\n=== Test 1: Entity Creation & Naming ===" << std::endl;
    uint32_t root = world.create_entity();
    world.set_entity_name(root, "Root");
    
    uint32_t child1 = world.create_entity();
    world.set_entity_name(child1, "Child1");
    
    uint32_t child2 = world.create_entity();
    world.set_entity_name(child2, "Child2");
    
    uint32_t grandchild = world.create_entity();
    world.set_entity_name(grandchild, "Grandchild");
    
    std::cout << "Created entities: root=" << root << ", child1=" << child1 
              << ", child2=" << child2 << ", grandchild=" << grandchild << std::endl;
    
    // Verify names
    const char* root_name = world.get_entity_name(root);
    if (!root_name || strcmp(root_name, "Root") != 0) {
        std::cerr << "FAILED: Root name not set correctly" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Entity naming works" << std::endl;
    
    // Test 2: Find entity by name
    std::cout << "\n=== Test 2: Find Entity by Name ===" << std::endl;
    uint32_t found_child1 = world.find_entity_by_name("Child1");
    if (found_child1 != child1) {
        std::cerr << "FAILED: find_entity_by_name returned " << found_child1 
                  << ", expected " << child1 << std::endl;
        return 1;
    }
    
    uint32_t not_found = world.find_entity_by_name("NonExistent");
    if (not_found != 0) {
        std::cerr << "FAILED: find_entity_by_name should return 0 for non-existent" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Entity lookup by name works" << std::endl;
    
    // Test 3: Parent-child relationships
    std::cout << "\n=== Test 3: Parent-Child Relationships ===" << std::endl;
    world.set_entity_parent(child1, root);
    world.set_entity_parent(child2, root);
    world.set_entity_parent(grandchild, child1);
    
    // Verify parent relationships
    if (world.get_entity_parent(child1) != root) {
        std::cerr << "FAILED: child1's parent is not root" << std::endl;
        return 1;
    }
    
    if (world.get_entity_parent(grandchild) != child1) {
        std::cerr << "FAILED: grandchild's parent is not child1" << std::endl;
        return 1;
    }
    
    // Verify children
    const std::vector<uint32_t>* root_children = world.get_entity_children(root);
    if (!root_children || root_children->size() != 2) {
        std::cerr << "FAILED: root should have 2 children" << std::endl;
        return 1;
    }
    
    const std::vector<uint32_t>* child1_children = world.get_entity_children(child1);
    if (!child1_children || child1_children->size() != 1) {
        std::cerr << "FAILED: child1 should have 1 child" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Parent-child relationships work" << std::endl;
    
    // Test 4: Transform propagation
    std::cout << "\n=== Test 4: Transform Propagation ===" << std::endl;
    
    // Set root transform: translate (10, 0, 0)
    world.set_entity_transform(root, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    
    // Set child1 transform: translate (5, 0, 0) relative to parent
    world.set_entity_transform(child1, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    
    // Set grandchild transform: translate (3, 0, 0) relative to parent
    world.set_entity_transform(grandchild, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    
    // Update world transforms
    world.update_world_transforms();
    
    // Check world matrices
    const float* root_matrix = world.get_entity_world_matrix(root);
    const float* child1_matrix = world.get_entity_world_matrix(child1);
    const float* grandchild_matrix = world.get_entity_world_matrix(grandchild);
    
    if (!root_matrix || !child1_matrix || !grandchild_matrix) {
        std::cerr << "FAILED: Could not get world matrices" << std::endl;
        return 1;
    }
    
    // Root world position should be (10, 0, 0)
    if (!float_equal(root_matrix[12], 10.0f) || 
        !float_equal(root_matrix[13], 0.0f) || 
        !float_equal(root_matrix[14], 0.0f)) {
        std::cerr << "FAILED: Root world position incorrect: (" 
                  << root_matrix[12] << ", " << root_matrix[13] << ", " << root_matrix[14] << ")" << std::endl;
        return 1;
    }
    
    // Child1 world position should be (10 + 5, 0, 0) = (15, 0, 0)
    if (!float_equal(child1_matrix[12], 15.0f) || 
        !float_equal(child1_matrix[13], 0.0f) || 
        !float_equal(child1_matrix[14], 0.0f)) {
        std::cerr << "FAILED: Child1 world position incorrect: (" 
                  << child1_matrix[12] << ", " << child1_matrix[13] << ", " << child1_matrix[14] << ")" << std::endl;
        std::cerr << "Expected: (15, 0, 0)" << std::endl;
        return 1;
    }
    
    // Grandchild world position should be (10 + 5 + 3, 0, 0) = (18, 0, 0)
    if (!float_equal(grandchild_matrix[12], 18.0f) || 
        !float_equal(grandchild_matrix[13], 0.0f) || 
        !float_equal(grandchild_matrix[14], 0.0f)) {
        std::cerr << "FAILED: Grandchild world position incorrect: (" 
                  << grandchild_matrix[12] << ", " << grandchild_matrix[13] << ", " << grandchild_matrix[14] << ")" << std::endl;
        std::cerr << "Expected: (18, 0, 0)" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Transform propagation works correctly" << std::endl;
    std::cout << "  Root world pos: (10, 0, 0)" << std::endl;
    std::cout << "  Child1 world pos: (15, 0, 0)" << std::endl;
    std::cout << "  Grandchild world pos: (18, 0, 0)" << std::endl;
    
    // Test 5: Incremental updates (dirty flag optimization)
    std::cout << "\n=== Test 5: Incremental Transform Updates ===" << std::endl;
    
    // Update only root transform
    world.set_entity_transform(root, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    world.update_world_transforms();
    
    child1_matrix = world.get_entity_world_matrix(child1);
    grandchild_matrix = world.get_entity_world_matrix(grandchild);
    
    // Child1 world position should be (20 + 5, 0, 0) = (25, 0, 0)
    if (!float_equal(child1_matrix[12], 25.0f)) {
        std::cerr << "FAILED: Incremental update didn't propagate correctly" << std::endl;
        return 1;
    }
    
    // Grandchild world position should be (20 + 5 + 3, 0, 0) = (28, 0, 0)
    if (!float_equal(grandchild_matrix[12], 28.0f)) {
        std::cerr << "FAILED: Incremental update didn't propagate to grandchild" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Incremental updates work correctly" << std::endl;
    
    // Test 6: Reparenting
    std::cout << "\n=== Test 6: Reparenting ===" << std::endl;
    
    // Move grandchild from child1 to child2
    world.set_entity_parent(grandchild, child2);
    
    if (world.get_entity_parent(grandchild) != child2) {
        std::cerr << "FAILED: Reparenting didn't work" << std::endl;
        return 1;
    }
    
    // Verify child1 has no children now
    child1_children = world.get_entity_children(child1);
    if (!child1_children || child1_children->size() != 0) {
        std::cerr << "FAILED: child1 should have 0 children after reparenting" << std::endl;
        return 1;
    }
    
    // Verify child2 has grandchild
    const std::vector<uint32_t>* child2_children = world.get_entity_children(child2);
    if (!child2_children || child2_children->size() != 1) {
        std::cerr << "FAILED: child2 should have 1 child after reparenting" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Reparenting works correctly" << std::endl;
    
    // Test 7: Bounding boxes (stub)
    std::cout << "\n=== Test 7: Bounding Boxes (Stub) ===" << std::endl;
    
    World::AABB bbox;
    if (!world.get_entity_bounding_box(root, bbox)) {
        std::cerr << "FAILED: get_entity_bounding_box returned false" << std::endl;
        return 1;
    }
    
    std::cout << "Bounding box for root: min(" << bbox.min_x << ", " << bbox.min_y << ", " << bbox.min_z 
              << ") max(" << bbox.max_x << ", " << bbox.max_y << ", " << bbox.max_z << ")" << std::endl;
    std::cout << "SUCCESS: Bounding box stub works" << std::endl;
    
    // Test 8: Deep hierarchy performance
    std::cout << "\n=== Test 8: Deep Hierarchy ===" << std::endl;
    
    uint32_t prev = world.create_entity();
    world.set_entity_name(prev, "Chain_Start");
    world.set_entity_transform(prev, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    
    // Create a chain of 100 entities
    for (int i = 0; i < 100; ++i) {
        uint32_t curr = world.create_entity();
        world.set_entity_parent(curr, prev);
        world.set_entity_transform(curr, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        prev = curr;
    }
    
    // Update all transforms
    world.update_world_transforms();
    
    // Check final entity position (should be at 100 in X)
    const float* final_matrix = world.get_entity_world_matrix(prev);
    if (!final_matrix) {
        std::cerr << "FAILED: Could not get final entity matrix" << std::endl;
        return 1;
    }
    
    if (!float_equal(final_matrix[12], 100.0f)) {
        std::cerr << "FAILED: Deep hierarchy propagation incorrect: " << final_matrix[12] 
                  << ", expected 100.0" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Deep hierarchy (100 levels) works correctly" << std::endl;
    std::cout << "  Final entity world pos X: " << final_matrix[12] << std::endl;
    
    // Test 9: Entity deletion with hierarchy
    std::cout << "\n=== Test 9: Entity Deletion with Hierarchy ===" << std::endl;
    
    uint32_t count_before = world.get_entity_count();
    world.destroy_entity(child1);
    uint32_t count_after = world.get_entity_count();
    
    if (count_after != count_before - 1) {
        std::cerr << "FAILED: Entity count not decremented correctly" << std::endl;
        return 1;
    }
    
    // Verify child1 is removed from root's children
    root_children = world.get_entity_children(root);
    if (!root_children || root_children->size() != 1) {
        std::cerr << "FAILED: child1 should be removed from root's children list" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Entity deletion with hierarchy works" << std::endl;
    
    // Test 10: Circular reference detection
    std::cout << "\n=== Test 10: Circular Reference Detection ===" << std::endl;
    
    uint32_t a = world.create_entity();
    uint32_t b = world.create_entity();
    uint32_t c = world.create_entity();
    
    world.set_entity_name(a, "A");
    world.set_entity_name(b, "B");
    world.set_entity_name(c, "C");
    
    // Create chain: A -> B -> C
    world.set_entity_parent(b, a);
    world.set_entity_parent(c, b);
    
    // Try to create cycle: A -> B -> C -> A (should be rejected)
    world.set_entity_parent(a, c);
    
    // Verify A still has no parent (cycle was rejected)
    if (world.get_entity_parent(a) != 0) {
        std::cerr << "FAILED: Circular reference was not detected" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Circular reference detection works" << std::endl;
    
    std::cout << "\n=== All Tests Passed ===" << std::endl;
    std::cout << "\nSummary:" << std::endl;
    std::cout << "✓ Entity naming and lookup works" << std::endl;
    std::cout << "✓ Parent-child relationships work" << std::endl;
    std::cout << "✓ Transform propagation works correctly" << std::endl;
    std::cout << "✓ Incremental updates (dirty flags) work" << std::endl;
    std::cout << "✓ Reparenting works" << std::endl;
    std::cout << "✓ Bounding box computation (stub) works" << std::endl;
    std::cout << "✓ Deep hierarchies (100 levels) work efficiently" << std::endl;
    std::cout << "✓ Entity deletion with hierarchy works" << std::endl;
    std::cout << "✓ Circular reference detection works" << std::endl;
    
    world.shutdown();
    
    return 0;
}
