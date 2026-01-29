# Asset Pipeline Implementation (Task C2)

## Overview

This document describes the implementation of the asset pipeline for Astraeus, including mesh loading, GPU upload queue, and rendering capabilities.

## Components

### 1. Mesh Loader (`engine/assets/MeshLoader.hpp`)

A simple OBJ file parser that supports:
- Vertex positions (v x y z)
- Vertex normals (vn nx ny nz)
- Texture coordinates (vt u v)
- Face definitions with v//vn format
- Automatic triangulation for polygons with more than 3 vertices

**Key Features:**
- Static load method: `MeshLoader::load_obj(filepath, out_mesh)`
- Returns loaded geometry as a `Mesh` object
- Handles various OBJ formats (with or without normals/texcoords)

### 2. GPU Upload Queue (`engine/assets/GPUUploadQueue.hpp`)

Manages deferred GPU resource uploads with staging buffers.

**Key Features:**
- **Deferred uploads**: Mesh data is queued and uploaded during render thread execution
- **Reference counting**: Multiple references to the same asset share GPU resources
- **Staged processing**: Configurable number of uploads per frame to avoid frame spikes
- **Safe cleanup**: Resources are only deleted when all references are released

**GPU Resources:**
- VAO (Vertex Array Object)
- VBO (Vertex Buffer Object) - stores interleaved vertex data
- IBO (Index Buffer Object) - stores indices

**Vertex Layout:**
```
location 0: position (vec3)
location 1: normal (vec3)
location 2: texcoord (vec2)
```

### 3. Asset Manager Updates (`engine/assets/AssetManager.hpp`)

Enhanced to support the new asset pipeline:

**New Features:**
- **Path-based caching**: Same file loaded multiple times returns the same asset ID
- **Automatic deduplication**: Prevents duplicate GPU buffers for identical assets
- **Reference tracking**: Tracks how many times each asset is loaded
- **Safe unloading**: Assets are only deleted when all references are released

**Key Methods:**
- `load_model(path)`: Load a mesh from disk, returns asset ID
- `unload_asset(asset_id)`: Release a reference to an asset
- `process_uploads()`: Process pending GPU uploads (called per frame)
- `get_gpu_mesh(asset_id)`: Get GPU resource for rendering
- `is_ready(asset_id)`: Check if asset is uploaded to GPU

### 4. Unlit Shader (`engine/renderer/UnlitShader.hpp`)

Simple shader for rendering meshes without complex lighting.

**Features:**
- Vertex and fragment shader implementation
- MVP (Model-View-Projection) matrix support
- Configurable flat color per mesh
- Simple diffuse lighting based on normals
- OpenGL 3.3 Core compatible

### 5. Mesh Render Pass (`engine/renderer/passes/MeshPass.hpp`)

Integrates mesh rendering into the render graph.

**Features:**
- Renders all entities with mesh assets
- Supports entity transforms (position, rotation, scale)
- Uses entity colors if available
- Computes MVP matrices per entity
- Depth testing enabled for proper 3D rendering

## Usage Example

```cpp
// Initialize engine
EngineContext engine(config);
engine.initialize();

// Get asset manager
AssetManager* asset_mgr = engine.get_asset_manager();

// Load a model
uint32_t cube_id = asset_mgr->load_model("path/to/cube.obj");

// Load the same model again (returns same ID, shares GPU resources)
uint32_t cube_id_2 = asset_mgr->load_model("path/to/cube.obj");
assert(cube_id == cube_id_2);

// Process uploads each frame
asset_mgr->process_uploads();

// Check if ready for rendering
if (asset_mgr->is_ready(cube_id)) {
    // Mesh is uploaded to GPU and ready to render
}

// Unload when done
asset_mgr->unload_asset(cube_id);    // ref_count: 2 -> 1
asset_mgr->unload_asset(cube_id_2);  // ref_count: 1 -> 0, GPU resources deleted
```

## Integration with Engine

1. **Engine Context**: Calls `asset_manager->process_uploads()` in `end_frame()`
2. **Render Graph**: MeshPass can be added to render loaded meshes
3. **World/Scene**: Entity system supports mesh rendering via asset IDs

## Acceptance Criteria

### ✓ No duplicate GPU buffers for identical assets
- Implemented via path-based caching in AssetManager
- Multiple loads of the same file return the same asset ID
- Reference counting ensures GPU resources are shared

### ✓ Assets can be unloaded safely
- Reference counting prevents premature deletion
- `unload_asset()` decrements ref count
- GPU resources deleted only when ref count reaches 0

### ✓ No dangling GPU handles after unload
- `release()` method in GPUUploadQueue checks ref count
- Resources are invalidated before deletion
- Accessing deleted assets returns nullptr

## Testing

### Unit Test (`examples/asset_unit_test.cpp`)

Tests without OpenGL context:
- OBJ file loading and parsing
- Mesh data structure validation
- Reference counting logic
- Format compatibility

### Integration Test (`examples/asset_test.cpp`)

Requires OpenGL context (full engine initialization):
- Complete asset loading pipeline
- GPU upload queue processing
- Reference counting with real GPU resources
- Safe unload verification

## Future Enhancements

1. **glTF Support**: Add glTF/GLB loader for more complex models
2. **Texture Loading**: Extend to support texture assets
3. **Material System**: Support multiple material types beyond unlit
4. **Async Loading**: Load meshes on background thread
5. **Compression**: Support compressed mesh formats
6. **LOD System**: Level-of-detail support for large scenes

## Technical Notes

- All implementations are header-only for maximum flexibility
- OpenGL 3.3 Core is the minimum requirement
- Vertex data is interleaved for optimal cache performance
- Upload queue processes 1 asset per frame by default (configurable)
