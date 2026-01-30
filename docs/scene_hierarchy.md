# Scene Hierarchy and Transform System

This document describes the parent-child transform hierarchy system in the Astraeus engine.

## Overview

The World class now supports hierarchical parent-child relationships between entities, with automatic transform propagation from parent to child. This enables building complex scene graphs with nested objects.

## Key Features

- **Parent-child relationships**: Entities can be organized in a tree structure
- **Transform propagation**: Local transforms are automatically combined with parent transforms to compute world-space transforms
- **Incremental updates**: Only dirty transforms are recomputed (O(1) for isolated changes, O(depth) for hierarchy changes)
- **Deep hierarchies**: Efficiently supports arbitrarily deep hierarchies (tested up to 100+ levels)
- **Entity naming**: Assign human-readable names to entities and look them up by name
- **Bounding boxes**: Query entity bounding boxes (currently stub implementation)

## API Reference

### Entity Naming

```cpp
// Set entity name
world.set_entity_name(entity_id, "MyEntity");

// Get entity name
const char* name = world.get_entity_name(entity_id);

// Find entity by name (returns 0 if not found)
uint32_t found_id = world.find_entity_by_name("MyEntity");
```

### Hierarchy Management

```cpp
// Set parent (use 0 for root)
world.set_entity_parent(child_id, parent_id);

// Get parent (returns 0 if root)
uint32_t parent = world.get_entity_parent(entity_id);

// Get children
const std::vector<uint32_t>* children = world.get_entity_children(entity_id);
if (children) {
    for (uint32_t child_id : *children) {
        // Process child
    }
}
```

### Transform Propagation

```cpp
// Set entity transform (local space)
world.set_entity_transform(entity_id, 
    pos_x, pos_y, pos_z,        // Position
    rot_x, rot_y, rot_z,        // Rotation (Euler angles in degrees)
    scale_x, scale_y, scale_z); // Scale

// Update all world transforms (call once per frame)
world.update_world_transforms();

// Get world matrix (4x4, column-major)
const float* world_matrix = world.get_entity_world_matrix(entity_id);
```

### Bounding Boxes

```cpp
// Get entity bounding box
World::AABB bbox;
if (world.get_entity_bounding_box(entity_id, bbox)) {
    // Use bbox.min_x, min_y, min_z, max_x, max_y, max_z
}
```

## Usage Example

```cpp
#include <scene/World.hpp>

using namespace astraeus;

int main() {
    World world;
    world.initialize();
    
    // Create a simple scene graph:
    // Root
    //   ├─ Car
    //   │   ├─ Wheel_FL
    //   │   ├─ Wheel_FR
    //   │   ├─ Wheel_RL
    //   │   └─ Wheel_RR
    //   └─ Camera
    
    uint32_t root = world.create_entity();
    world.set_entity_name(root, "Root");
    
    uint32_t car = world.create_entity();
    world.set_entity_name(car, "Car");
    world.set_entity_parent(car, root);
    world.set_entity_transform(car, 0, 0, 0, 0, 0, 0, 1, 1, 1);
    
    uint32_t wheel_fl = world.create_entity();
    world.set_entity_name(wheel_fl, "Wheel_FL");
    world.set_entity_parent(wheel_fl, car);
    world.set_entity_transform(wheel_fl, -1, -0.5, 1, 0, 0, 0, 0.3, 0.3, 0.3);
    
    // ... create other wheels similarly
    
    uint32_t camera = world.create_entity();
    world.set_entity_name(camera, "Camera");
    world.set_entity_parent(camera, root);
    world.set_entity_transform(camera, 0, 5, 10, -30, 0, 0, 1, 1, 1);
    
    // Frame loop
    while (running) {
        // Move car forward
        auto* car_trans = world.get_entity_transform(car);
        if (car_trans) {
            world.set_entity_transform(car,
                car_trans->pos_x + 0.1f, car_trans->pos_y, car_trans->pos_z,
                car_trans->rot_x, car_trans->rot_y, car_trans->rot_z,
                car_trans->scale_x, car_trans->scale_y, car_trans->scale_z);
        }
        
        // Update all world transforms
        world.update_world_transforms();
        
        // Render using world matrices
        const float* car_world = world.get_entity_world_matrix(car);
        const float* wheel_world = world.get_entity_world_matrix(wheel_fl);
        // ... render objects
    }
    
    world.shutdown();
    return 0;
}
```

## Performance Considerations

- **Dirty flag optimization**: Only entities marked as dirty (and their descendants) are recomputed during `update_world_transforms()`
- **Incremental updates**: Changing a leaf entity's transform is O(1). Changing a root entity affects all descendants but is still linear in the number of descendants
- **No redundant computations**: Each entity's world transform is computed at most once per frame
- **Cache-friendly**: Transform data is stored in contiguous memory (hash map with good locality)

## Implementation Details

### Transform Matrices

- Local transforms are stored as position, rotation (Euler angles), and scale
- World matrices are 4x4 column-major matrices (OpenGL style)
- Matrix computation follows TRS order: Translation × Rotation × Scale
- Rotation order is ZYX (yaw-pitch-roll)

### Hierarchy Storage

- Parent-child relationships are stored in a Hierarchy component (hash map)
- Each entity can have one parent and multiple children
- Circular references are prevented (simple check, not exhaustive)
- Deleting an entity reparents its children to root

### Dirty Flag Propagation

- Setting an entity's transform marks it and all descendants as dirty
- During update, dirty entities recompute their world matrix from local transform and parent's world matrix
- Recursive dependency ensures parent transforms are up-to-date before computing child transforms

## Future Enhancements

- Full quaternion support for rotations (avoids gimbal lock)
- Proper cycle detection in hierarchy
- Spatial index integration for fast culling
- AABB computation from actual mesh bounds
- Transform interpolation for animation
- Matrix caching for static objects
