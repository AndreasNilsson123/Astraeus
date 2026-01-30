# Camera System Documentation

## Overview

The Astraeus engine now includes a comprehensive camera system with support for:
- **Multiple camera types**: Perspective and Orthographic projections
- **Component-based architecture**: Cameras can be attached to any entity
- **Frustum culling**: Built-in frustum extraction for efficient culling
- **Renderer-friendly data**: Structured uniform packs for GPU upload

## Core Components

### 1. CameraComponent (`scene/CameraComponent.hpp`)

A component that can be attached to entities to give them camera capabilities.

**Features:**
- Perspective and orthographic projection support
- Near/far plane configuration
- Field of view (FOV) for perspective cameras
- Exposure placeholder for HDR rendering
- Computed view, projection, and view-projection matrices
- Built-in frustum for culling

**Example:**
```cpp
CameraComponent camera;
camera.projection_type = CameraProjectionType::Perspective;
camera.fov_degrees = 60.0f;
camera.near_plane = 0.1f;
camera.far_plane = 1000.0f;
```

### 2. Frustum (`scene/CameraComponent.hpp`)

A 6-plane frustum representation for culling geometry.

**Methods:**
- `contains_point(x, y, z)` - Test if a point is inside the frustum
- `intersects_sphere(x, y, z, radius)` - Test sphere intersection
- `intersects_aabb(min_x, min_y, min_z, max_x, max_y, max_z)` - Test AABB intersection

**Example:**
```cpp
const Frustum& frustum = camera.frustum;
if (frustum.intersects_sphere(obj_x, obj_y, obj_z, obj_radius)) {
    // Object is visible, render it
}
```

### 3. CameraSystem (`scene/CameraSystem.hpp`)

Static utility class for camera operations.

**Key Methods:**
- `update_view_matrix()` - Build view matrix from position and target
- `update_projection_matrix()` - Build projection matrix with aspect ratio
- `extract_frustum()` - Extract frustum planes from view-projection matrix
- `update_camera()` - Complete camera update (view, projection, VP, frustum)
- `build_camera_uniforms()` - Pack camera data for renderer
- `set_perspective()` - Configure perspective projection
- `set_orthographic()` - Configure orthographic projection

**Example:**
```cpp
// Update camera from position and target
CameraSystem::update_camera(camera, 
    eye_x, eye_y, eye_z,
    target_x, target_y, target_z,
    up_x, up_y, up_z,
    aspect_ratio);

// Build uniforms for rendering
CameraUniforms uniforms;
CameraSystem::build_camera_uniforms(camera, cam_x, cam_y, cam_z, uniforms);
```

### 4. CameraUniforms (`scene/CameraComponent.hpp`)

Renderer-friendly structure for GPU upload.

**Contents:**
- View, projection, and view-projection matrices
- Camera world position
- Camera forward direction
- Near and far planes
- FOV and exposure values

## World Integration

The `World` class now supports camera components on entities:

### Creating a Camera Entity

```cpp
World world;
world.initialize();

// Create an entity with a camera
uint32_t camera_entity = world.create_entity();
world.set_entity_camera(camera_entity, CameraProjectionType::Perspective);

// Configure the camera
CameraComponent* camera = world.get_entity_camera(camera_entity);
camera->fov_degrees = 45.0f;
camera->near_plane = 0.5f;
camera->far_plane = 500.0f;
```

### Setting Active Camera

```cpp
// Set this camera as the active one for rendering
world.set_active_camera(camera_entity);

// Get the active camera
CameraComponent* active_cam = world.get_active_camera();
```

### Positioning the Camera

Cameras use the entity's transform:

```cpp
// Position the camera in the world
world.set_entity_transform(camera_entity,
    10.0f, 5.0f, 10.0f,  // position
    0.0f, 45.0f, 0.0f,   // rotation (degrees)
    1.0f, 1.0f, 1.0f);   // scale

// Update world transforms
world.update_world_transforms();

// Update camera matrices from entity transform
float aspect_ratio = width / height;
world.update_entity_camera(camera_entity, aspect_ratio);
```

### Using Camera for Rendering

```cpp
// Get camera uniforms for shader upload
CameraUniforms uniforms;
world.build_camera_uniforms(camera_entity, uniforms);

// Access matrices
const float* view_proj = uniforms.view_projection_matrix;
const float* camera_pos = uniforms.camera_position;

// Frustum culling
const CameraComponent* camera = world.get_entity_camera(camera_entity);
for (uint32_t entity_id : world.get_renderable_entities()) {
    World::AABB aabb;
    if (world.get_entity_bounding_box(entity_id, aabb)) {
        if (camera->frustum.intersects_aabb(
            aabb.min_x, aabb.min_y, aabb.min_z,
            aabb.max_x, aabb.max_y, aabb.max_z)) {
            // Render this entity
        }
    }
}
```

## Projection Types

### Perspective Projection

```cpp
CameraSystem::set_perspective(camera, 
    60.0f,    // FOV in degrees
    0.1f,     // near plane
    1000.0f); // far plane
```

### Orthographic Projection

```cpp
CameraSystem::set_orthographic(camera,
    -10.0f, 10.0f,   // left, right
    -10.0f, 10.0f,   // bottom, top
    0.1f, 100.0f);   // near, far
```

## Multiple Cameras

You can have multiple camera entities and switch between them:

```cpp
// Create multiple cameras
uint32_t cam1 = world.create_entity();
uint32_t cam2 = world.create_entity();

world.set_entity_camera(cam1, CameraProjectionType::Perspective);
world.set_entity_camera(cam2, CameraProjectionType::Orthographic);

// Position them differently
world.set_entity_transform(cam1, 10.0f, 5.0f, 10.0f, 0, 0, 0, 1, 1, 1);
world.set_entity_transform(cam2, 0.0f, 20.0f, 0.0f, 0, 0, 0, 1, 1, 1);

// Update all cameras at once
world.update_world_transforms();
world.update_all_cameras(aspect_ratio);

// Switch active camera
world.set_active_camera(cam2);
```

## Backward Compatibility

The legacy `Camera` class (orbit camera) is still available in World for backward compatibility:

```cpp
// Legacy camera API still works
world.set_camera(eye_x, eye_y, eye_z, target_x, target_y, target_z, up_x, up_y, up_z);
world.set_camera_projection(fov, near_plane, far_plane);
world.update_camera(aspect_ratio);

const Camera& cam = world.get_camera();
const float* vp_matrix = cam.get_view_projection_matrix();
```

## Performance Notes

- Camera matrices are only recomputed when dirty (position/projection changed)
- Frustum extraction is done once per camera update
- Multiple cameras can be updated efficiently in batch
- Entity transforms drive camera position automatically

## Testing

See `engine/examples/camera_system_test.cpp` and `engine/examples/camera_component_test.cpp` for comprehensive test examples.

## Future Extensions

The camera system is designed to be extensible:
- Add more projection types (fisheye, stereoscopic, etc.)
- Extend CameraUniforms with more rendering parameters
- Add camera animation/interpolation utilities
- Implement advanced culling strategies
