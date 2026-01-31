# glTF Loader Usage Guide

## Overview

The Astraeus glTF loader (`GLTFLoader.hpp`) provides comprehensive support for loading glTF 2.0 assets, including meshes, materials, and textures.

## Important: Implementation Guards

The glTF loader is header-only but uses tinygltf, which requires careful handling to avoid multiple definition errors.

### Correct Usage

**In ONE .cpp file only**, define the implementation before including:

```cpp
#define ASTRAEUS_GLTF_LOADER_IMPLEMENTATION
#include <assets/GLTFLoader.hpp>
```

**In all other files**, just include normally:

```cpp
#include <assets/GLTFLoader.hpp>
```

### Why This Matters

The tinygltf library and stb_image are header-only but include function implementations. Without the guard, including `GLTFLoader.hpp` in multiple files will cause linker errors about multiply defined symbols.

## Basic Usage

### Loading a glTF Model

```cpp
#include <assets/GLTFLoader.hpp>

GLTFModel model;
if (GLTFLoader::load_gltf("path/to/model.gltf", model)) {
    std::cout << "Loaded " << model.primitives.size() << " primitives\n";
    std::cout << "Materials: " << model.materials.size() << "\n";
    std::cout << "Textures: " << model.textures.size() << "\n";
}
```

### Accessing Mesh Data

```cpp
// Iterate over all primitives
for (const auto& primitive : model.primitives) {
    const Mesh& mesh = primitive.mesh;
    
    std::cout << "Primitive: " << primitive.name << "\n";
    std::cout << "  Vertices: " << mesh.get_vertex_count() << "\n";
    std::cout << "  Indices: " << mesh.get_index_count() << "\n";
    std::cout << "  Material: " << primitive.material_index << "\n";
}
```

### Accessing Materials

```cpp
// Get material for a primitive
const GLTFPrimitive& prim = model.primitives[0];
if (prim.material_index >= 0 && prim.material_index < model.materials.size()) {
    const GLTFMaterial& mat = model.materials[prim.material_index];
    
    // Base color
    float r = mat.base_color_factor[0];
    float g = mat.base_color_factor[1];
    float b = mat.base_color_factor[2];
    float a = mat.base_color_factor[3];
    
    // PBR properties
    float metallic = mat.metallic_factor;
    float roughness = mat.roughness_factor;
    
    // Check for textures
    if (mat.base_color_texture_index >= 0) {
        const Texture& tex = model.textures[mat.base_color_texture_index];
        std::cout << "Base color texture: " 
                  << tex.width << "x" << tex.height << "\n";
    }
}
```

### Accessing Textures

```cpp
// Iterate over textures
for (const auto& texture : model.textures) {
    std::cout << "Texture: " << texture.width << "x" << texture.height 
              << " (" << texture.channels << " channels)\n";
    
    // Access pixel data
    const std::vector<uint8_t>& pixels = texture.data;
    
    // Upload to GPU (see GPUUploadQueue for mesh example)
    // TODO: Texture GPU upload not yet implemented
}
```

## Using with AssetManager

The AssetManager automatically detects glTF files by extension:

```cpp
AssetManager* asset_mgr = engine.get_asset_manager();

// Load glTF (auto-detected by .gltf or .glb extension)
uint32_t model_id = asset_mgr->load_model("assets/character.gltf");

// Wait for upload
asset_mgr->process_uploads();

// Check if ready
if (asset_mgr->is_ready(model_id)) {
    const GPUMesh* gpu_mesh = asset_mgr->get_gpu_mesh(model_id);
    // Render...
}
```

**Note:** AssetManager currently uses only the first primitive from a glTF file. Multi-primitive support is planned for a future update.

## Supported Features

### ✅ Fully Supported
- glTF 2.0 JSON (.gltf) and binary (.glb) formats
- Multiple meshes per file
- Multiple primitives per mesh
- Vertex positions, normals, texture coordinates
- Triangle meshes
- PBR metallic-roughness materials
- Material base color, metallic, and roughness factors
- Texture references (base color, metallic-roughness, normal, occlusion, emissive)
- Embedded textures
- External texture files
- Alpha modes (opaque, mask, blend)
- Double-sided materials

### ❌ Not Yet Supported
- Non-triangle primitives (lines, points)
- Vertex colors
- Multiple texture coordinate sets
- Skeletal animations
- Morph targets
- Skins
- Multiple scenes
- glTF extensions

## File Format Support

### glTF (.gltf)
- JSON format
- Human-readable
- Separate .bin files for binary data
- Separate texture files

### Binary glTF (.glb)
- Binary container format
- Single file
- More compact
- Recommended for production

## Performance Considerations

1. **Binary Format**: Use .glb for faster loading and smaller file size
2. **Texture Size**: Large textures increase memory usage
3. **Primitive Count**: Many primitives may impact performance
4. **Caching**: AssetManager caches based on URI+hash

## Troubleshooting

### Linker Errors (Multiple Definitions)

**Problem**: `multiple definition of 'stbi_load'` or similar errors

**Solution**: Ensure `ASTRAEUS_GLTF_LOADER_IMPLEMENTATION` is defined in only ONE .cpp file

### Failed to Load File

**Problem**: `Failed to load glTF file`

**Solution**: 
- Check file path is correct
- Ensure file format is glTF 2.0
- Check for parse errors in console output
- Verify external resources (.bin, textures) are in the same directory

### No Primitives Loaded

**Problem**: Model loads but has 0 primitives

**Solution**:
- Check mesh uses triangle topology (mode 4 in glTF)
- Other primitive types are currently skipped
- Verify mesh has POSITION attribute

### Texture Not Loaded

**Problem**: Texture data is empty

**Solution**:
- Ensure stb_image supports the image format
- Check texture file path is correct
- Verify texture is embedded or in the same directory as .gltf

## Examples

See `engine/examples/gltf_loader_test.cpp` for comprehensive usage examples.

## Future Improvements

- [ ] Multi-primitive support in AssetManager
- [ ] GPU texture upload
- [ ] Animation playback
- [ ] Skeletal animation
- [ ] Morph targets
- [ ] glTF extensions support
