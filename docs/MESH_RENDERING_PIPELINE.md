# Mesh Rendering Pipeline v1

## Overview

The Astraeus mesh rendering pipeline v1 provides a flexible and efficient system for rendering static meshes with various vertex formats. The system supports multiple vertex attribute layouts and implements batching to minimize state changes.

## Features

### 1. Flexible Vertex Formats

The system supports multiple standard vertex formats:

- **P3N3T2** - Position + Normal + TexCoord (32 bytes/vertex)
- **P3N3T2C4** - Position + Normal + TexCoord + Color (48 bytes/vertex)
- **P3C4** - Position + Color (28 bytes/vertex)
- **P3N3T2TB3** - Position + Normal + TexCoord + Tangent + Bitangent (56 bytes/vertex)

### 2. Format-Aware GPU Upload

The `GPUUploadQueue` automatically configures vertex array objects (VAOs) based on the vertex format, setting up appropriate vertex attribute pointers for each attribute type.

### 3. Material and Mesh Batching

The `StaticMeshPass` implements efficient batching:
- Groups draw calls by material to minimize material binds
- Groups by mesh within each material to minimize VAO binds
- Tracks statistics for optimization analysis

## Architecture

### Core Components

```
┌─────────────────────────────────────────────────────────┐
│                  Mesh Rendering Pipeline                │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌──────────────┐      ┌─────────────────┐            │
│  │VertexFormat  │──────│ StandardFormats │            │
│  └──────────────┘      └─────────────────┘            │
│         │                                               │
│         ▼                                               │
│  ┌──────────────┐      ┌─────────────────┐            │
│  │    Mesh      │──────│  MeshUploader   │            │
│  └──────────────┘      └─────────────────┘            │
│         │                                               │
│         ▼                                               │
│  ┌──────────────┐      ┌─────────────────┐            │
│  │GPUUploadQueue│──────│    GPUMesh      │            │
│  └──────────────┘      └─────────────────┘            │
│         │                                               │
│         ▼                                               │
│  ┌──────────────┐      ┌─────────────────┐            │
│  │StaticMeshPass│──────│  MaterialLib    │            │
│  └──────────────┘      └─────────────────┘            │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Class Responsibilities

#### `VertexFormat`
- Defines vertex attribute layout
- Stores stride and attribute descriptors
- Provides attribute queries (has_attribute, get_attribute)

#### `MeshUploader`
- Factory methods for creating meshes with different formats
- Helper utilities (create_cube, create_colored_triangle)
- Converts typed vertex data to raw float arrays

#### `GPUMesh`
- Represents a mesh uploaded to GPU
- Stores OpenGL handles (VAO, VBO, IBO)
- Includes vertex format descriptor

#### `GPUUploadQueue`
- Manages asynchronous mesh uploads
- Creates VAOs with format-aware attribute setup
- Handles reference counting for shared meshes

#### `StaticMeshPass`
- Renders static meshes efficiently
- Implements material and mesh batching
- Tracks rendering statistics

## Usage Examples

### Creating Meshes

```cpp
#include "assets/MeshUploader.hpp"

// Create a simple cube
Mesh cube = MeshUploader::create_cube(1.0f);

// Create a colored triangle
Mesh triangle = MeshUploader::create_colored_triangle();

// Create a custom mesh with color
std::vector<Vertex_P3N3T2C4> vertices = {
    {0.0f, 1.0f, 0.0f, 0,0,1, 0.5f,0.0f, 1,0,0,1},  // Red
    {-1.0f,-1.0f,0.0f, 0,0,1, 0.0f,1.0f, 0,1,0,1},  // Green
    {1.0f,-1.0f, 0.0f, 0,0,1, 1.0f,1.0f, 0,0,1,1}   // Blue
};
Mesh colored = MeshUploader::create_P3N3T2C4(vertices, {0, 1, 2});
```

### Querying Vertex Formats

```cpp
const VertexFormat& format = mesh.get_vertex_format();

// Check for specific attributes
if (format.has_attribute(VertexAttributeType::Color)) {
    std::cout << "Mesh has vertex colors" << std::endl;
}

// Get attribute details
const VertexAttribute* pos = format.get_attribute(VertexAttributeType::Position);
if (pos) {
    std::cout << "Position at location " << pos->gl_location 
              << ", offset " << pos->offset << std::endl;
}

// Get stride
uint32_t stride = format.get_stride();
std::cout << "Vertex size: " << stride << " bytes" << std::endl;
```

### Uploading to GPU

```cpp
GPUUploadQueue upload_queue;
upload_queue.initialize();

// Enqueue mesh for upload
upload_queue.enqueue_upload(mesh_id, mesh);

// Process uploads (call once per frame on render thread)
upload_queue.process_uploads(1);

// Get GPU mesh for rendering
const GPUMesh* gpu_mesh = upload_queue.get_gpu_mesh(mesh_id);
if (gpu_mesh && gpu_mesh->is_valid()) {
    // Mesh is ready for rendering
    glBindVertexArray(gpu_mesh->vao);
    // ... render ...
}
```

### Using StaticMeshPass

```cpp
StaticMeshPass mesh_pass(asset_manager, material_library);
mesh_pass.initialize(render_device);

// Execute pass (call once per frame)
mesh_pass.execute(render_device, world);
```

## Vertex Format Specifications

### P3N3T2 (Position + Normal + TexCoord)

```
Offset  | Size | Type     | Location | Usage
--------|------|----------|----------|------------------
0       | 12   | vec3     | 0        | Position (x,y,z)
12      | 12   | vec3     | 1        | Normal (nx,ny,nz)
24      | 8    | vec2     | 2        | TexCoord (u,v)
--------|------|----------|----------|------------------
Total: 32 bytes
```

### P3N3T2C4 (Position + Normal + TexCoord + Color)

```
Offset  | Size | Type     | Location | Usage
--------|------|----------|----------|------------------
0       | 12   | vec3     | 0        | Position (x,y,z)
12      | 12   | vec3     | 1        | Normal (nx,ny,nz)
24      | 8    | vec2     | 2        | TexCoord (u,v)
32      | 16   | vec4     | 3        | Color (r,g,b,a)
--------|------|----------|----------|------------------
Total: 48 bytes
```

### P3C4 (Position + Color)

```
Offset  | Size | Type     | Location | Usage
--------|------|----------|----------|------------------
0       | 12   | vec3     | 0        | Position (x,y,z)
12      | 16   | vec4     | 1        | Color (r,g,b,a)
--------|------|----------|----------|------------------
Total: 28 bytes
```

### P3N3T2TB3 (Position + Normal + TexCoord + Tangent + Bitangent)

```
Offset  | Size | Type     | Location | Usage
--------|------|----------|----------|------------------
0       | 12   | vec3     | 0        | Position (x,y,z)
12      | 12   | vec3     | 1        | Normal (nx,ny,nz)
24      | 8    | vec2     | 2        | TexCoord (u,v)
32      | 12   | vec3     | 3        | Tangent (tx,ty,tz)
44      | 12   | vec3     | 4        | Bitangent (bx,by,bz)
--------|------|----------|----------|------------------
Total: 56 bytes
```

## Batching Strategy

The `StaticMeshPass` uses a two-level batching strategy:

1. **Primary Sort: By Material**
   - Minimizes expensive material binds and shader switches
   - All objects with the same material are grouped together

2. **Secondary Sort: By Mesh**
   - Minimizes VAO binds within each material group
   - Objects with the same mesh are rendered sequentially

This approach significantly reduces the number of state changes compared to naive rendering.

### Statistics

The pass tracks:
- `draw_calls_submitted_` - Total GL draw calls issued
- `material_binds_` - Number of material state changes
- `vao_binds_` - Number of VAO binds

These can be used for performance analysis and optimization.

## Performance Considerations

### Memory Layout

All vertex formats use interleaved data (not structure-of-arrays) because:
- Better cache locality for vertex processing
- Simpler to upload and manage
- More compatible with modern GPU architectures

### Upload Strategy

- Uploads are processed incrementally (configurable per frame)
- Supports reference counting for shared meshes
- GPU resources are only deleted when ref count reaches zero

### Future Optimizations

Potential improvements for v2:
- Instanced rendering for identical meshes
- Multi-draw indirect for reduced CPU overhead
- Texture batching/atlasing
- Level-of-detail (LOD) support
- Frustum culling

## Testing

Run the mesh rendering test to verify the system:

```bash
cd build
./bin/mesh_rendering_test
```

This test validates:
- Vertex format creation
- Attribute layout and queries
- MeshUploader utilities
- Format-aware GPU upload

## Summary

The mesh rendering pipeline v1 provides:

✓ **Multiple vertex formats** - Support for various attribute combinations  
✓ **Format-aware GPU upload** - Automatic VAO configuration  
✓ **Efficient batching** - Material and mesh sorting  
✓ **Clean API** - Easy to use factory methods and utilities  
✓ **Extensible design** - Ready for future enhancements  

The system is production-ready for rendering static meshes with multiple materials and vertex formats.
