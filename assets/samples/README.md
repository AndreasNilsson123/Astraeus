# Sample Assets for Astraeus Engine

This directory contains sample 3D models for testing the asset pipeline.

## Supported Formats

- **OBJ**: Wavefront OBJ format (legacy, simple geometry only)
- **glTF 2.0**: Modern 3D asset format with full PBR materials and textures

## Sample Models

### cube.obj (Legacy)
Simple cube model in OBJ format. Located in `engine/examples/cube.obj`.
- 8 vertices, 12 triangles
- Includes normals
- No textures

### Future Samples

The following sample assets should be added:

1. **cube.gltf / cube.glb**
   - Simple cube with PBR material
   - Base color texture
   - Single mesh, single primitive

2. **multi_mesh.gltf**
   - Multiple meshes in one file
   - Tests multi-mesh loading

3. **textured_sphere.gltf**
   - Sphere with UV-mapped texture
   - Base color + normal map
   - Tests texture loading

## Adding Your Own Assets

To add custom models:

1. Export from Blender/Maya/etc. as glTF 2.0
2. Place .gltf/.glb file in this directory
3. Load using `AssetManager::load_model("assets/samples/your_model.gltf")`

## glTF Export Guidelines

For best compatibility:

- Use glTF 2.0 format
- Include normals and texture coordinates
- Use triangulated meshes
- Embed textures or use relative paths
- PBR Metallic-Roughness workflow preferred

## Notes

- Binary glTF (.glb) is more compact than JSON glTF (.gltf)
- External resources (textures, bins) should be in the same directory
- Asset caching is automatic based on URI + content hash
