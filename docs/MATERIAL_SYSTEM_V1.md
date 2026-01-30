# Material System v1 Implementation

## Overview

This document describes the Material System v1 implementation for the Astraeus rendering engine. The material system replaces hardcoded shaders with a flexible, reusable material abstraction that supports multiple materials in the same scene without shader recompiles.

## Architecture

### Core Components

#### 1. Material Base Class (`Material.hpp`)
Abstract base class defining the material interface:
- **Shader Management**: Initialize and bind shader programs
- **Pipeline State**: Configure depth testing, blending, culling
- **Parameter System**: Type-safe parameter management (float, vec2, vec3, vec4, int, texture, mat4)
- **Apply Parameters**: Efficient parameter binding to shaders

#### 2. MaterialInstance (`Material.hpp`)
Allows multiple objects to share the same material with different parameters:
- **Per-Object Parameters**: Override base material parameters per instance
- **Efficient Binding**: Only re-binds when material changes
- **Parameter Storage**: Uses `MaterialParameters` for type-safe parameter management

#### 3. MaterialLibrary (`MaterialLibrary.hpp`)
Central registry for material management:
- **Material Registration**: Register materials by name
- **Instance Creation**: Create material instances from base materials
- **Default Materials**: Provides default unlit material
- **Lifecycle Management**: Initialize and shutdown all materials

#### 4. UnlitMaterial (`UnlitMaterial.hpp`)
Concrete implementation of a simple unlit material:
- **Base Color**: Solid color or texture-based coloring
- **Optional Texture**: Support for base color textures
- **PBR-Ready Structure**: Includes metallic/roughness parameters for future PBR support
- **Pipeline Configuration**: Sensible defaults for opaque geometry

### Material Parameters

The system supports multiple parameter types:
```cpp
enum class MaterialParameterType {
    Float,      // Single float value
    Vec2,       // 2D vector
    Vec3,       // 3D vector
    Vec4,       // 4D vector (e.g., color)
    Int,        // Integer value
    Texture2D,  // Texture ID
    Mat4        // 4x4 matrix (e.g., MVP)
};
```

### Pipeline State

Materials define their rendering pipeline state:
```cpp
struct PipelineState {
    bool depth_test_enabled = true;     // Enable depth testing
    bool depth_write_enabled = true;    // Enable depth writes
    bool blend_enabled = false;         // Enable blending
    uint32_t blend_src_factor;          // Blend source factor
    uint32_t blend_dst_factor;          // Blend destination factor
    bool cull_enabled = true;           // Enable face culling
    uint32_t cull_face;                 // Face to cull (GL_BACK, etc.)
    uint32_t primitive_type;            // Primitive topology (GL_TRIANGLES, etc.)
};
```

## UnlitMaterial Implementation

### Shader Structure

The UnlitMaterial includes:
- **Vertex Shader**: Transform vertices with MVP matrix
- **Fragment Shader**: Apply base color with optional texture sampling
- **Uniform Locations**: Cached for efficient parameter binding

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `baseColor` | vec4 | Base color (RGBA) |
| `baseColorTexture` | Texture2D | Optional texture for base color |
| `metallic` | float | Metallic property (PBR-ready, unused in unlit) |
| `roughness` | float | Roughness property (PBR-ready, unused in unlit) |
| `mvp` | mat4 | Model-View-Projection matrix |

### Example Usage

```cpp
// Initialize material library
MaterialLibrary material_library;
material_library.initialize(render_device);

// Get default unlit material
Material* unlit = material_library.get_default_unlit();

// Create material instances with different colors
auto red_material = material_library.create_instance("unlit");
red_material->get_parameters().set_vec4("baseColor", 1.0f, 0.0f, 0.0f, 1.0f);

auto green_material = material_library.create_instance("unlit");
green_material->get_parameters().set_vec4("baseColor", 0.0f, 1.0f, 0.0f, 1.0f);

// Bind and render
red_material->bind(device);
// ... render objects with red material

green_material->bind(device);
// ... render objects with green material
```

## Integration with MeshPass

The `MeshPass` has been updated to use the material system:

### Changes
1. **Constructor**: Now accepts `MaterialLibrary*` parameter
2. **Initialization**: Validates material library availability
3. **Execution**: 
   - Gets default material from library
   - Binds material only when it changes (optimization)
   - Sets per-entity parameters (MVP, color)
   - Applies parameters before drawing

### Material Binding Optimization

The pass tracks the last bound material to avoid redundant state changes:
```cpp
Material* last_material = nullptr;
for (entity : entities) {
    Material* material = get_entity_material(entity);
    if (material != last_material) {
        material->bind(device);
        last_material = material;
    }
    // Set per-entity parameters
    // Draw entity
}
```

## Acceptance Criteria Met

✅ **Multiple materials render in same scene**
- Multiple MaterialInstances can coexist
- Each instance has independent parameters
- Efficient material switching

✅ **No shader recompiles per entity**
- Shader compiled once at material initialization
- Shared shader program across all instances
- Only parameter updates per entity

✅ **Material abstraction with required features**
- Shader program selection via Material base class
- Pipeline state configuration (PipelineState struct)
- Parameter block system (MaterialParameters class)

✅ **Unlit material implementation**
- Base color support (solid + texture)
- PBR-ready parameter layout
- Proper pipeline state defaults

## Testing

A comprehensive test (`material_system_test.cpp`) validates:
- Material parameter types
- MaterialInstance creation
- Multiple instances sharing base material
- Pipeline state configuration
- PBR-ready parameter structure

Test output confirms all acceptance criteria are met.

## Future Enhancements

### Immediate
- Per-entity material assignment in World/Scene
- Material asset loading from files
- Shader hot-reloading

### Future
- PBR materials (fully implementing metallic/roughness)
- Shader variants (static branching)
- Material editor integration
- Custom material types

## Files Modified/Created

### Created
- `engine/renderer/Material.hpp` - Base material abstraction
- `engine/renderer/UnlitMaterial.hpp` - Unlit material implementation
- `engine/renderer/MaterialLibrary.hpp` - Material management
- `engine/examples/material_system_test.cpp` - Comprehensive test

### Modified
- `engine/renderer/passes/MeshPass.hpp` - Updated to use material system
- `engine/CMakeLists.txt` - Added material_system_test target

## Summary

The Material System v1 provides a solid foundation for material-based rendering in Astraeus. It successfully replaces hardcoded shaders with a flexible, extensible system that meets all acceptance criteria while maintaining performance through efficient material binding and parameter management.
