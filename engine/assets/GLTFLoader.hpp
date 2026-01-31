#ifndef ASTRAEUS_GLTF_LOADER_HPP
#define ASTRAEUS_GLTF_LOADER_HPP

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "../geometry/Mesh.hpp"
#include "Texture.hpp"

// tinygltf requires these defines before inclusion
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../third_party/tinygltf/tiny_gltf.h"

namespace astraeus {

/**
 * Material data extracted from glTF
 */
struct GLTFMaterial {
    std::string name;
    
    // PBR Metallic-Roughness parameters
    float base_color_factor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float metallic_factor = 1.0f;
    float roughness_factor = 1.0f;
    
    // Texture indices (-1 if not present)
    int base_color_texture_index = -1;
    int metallic_roughness_texture_index = -1;
    int normal_texture_index = -1;
    int occlusion_texture_index = -1;
    int emissive_texture_index = -1;
    
    float emissive_factor[3] = {0.0f, 0.0f, 0.0f};
    
    // Alpha mode
    enum class AlphaMode {
        Opaque,
        Mask,
        Blend
    };
    AlphaMode alpha_mode = AlphaMode::Opaque;
    float alpha_cutoff = 0.5f;
    
    bool double_sided = false;
};

/**
 * Mesh primitive data extracted from glTF
 * A glTF mesh can have multiple primitives, each with its own material
 */
struct GLTFPrimitive {
    Mesh mesh;                          // Vertex and index data
    int material_index = -1;            // Index into materials array
    std::string name;
};

/**
 * Complete glTF model data
 */
struct GLTFModel {
    std::vector<GLTFPrimitive> primitives;  // All mesh primitives
    std::vector<GLTFMaterial> materials;    // Material data
    std::vector<Texture> textures;          // Texture data
    std::string name;
    
    bool is_valid() const {
        return !primitives.empty();
    }
};

/**
 * GLTFLoader - Load 3D models from glTF 2.0 files
 * 
 * Supports:
 * - Multiple meshes and primitives
 * - Materials (PBR metallic-roughness)
 * - Textures (embedded and external)
 * - Triangle primitives only (converts other types)
 */
class GLTFLoader {
public:
    /**
     * Load a glTF model from file
     * @param filepath Path to .gltf or .glb file
     * @param out_model Output model structure
     * @return true on success, false on failure
     */
    static bool load_gltf(const std::string& filepath, GLTFModel& out_model) {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err, warn;
        
        bool ret = false;
        
        // Determine file type and load
        if (filepath.substr(filepath.length() - 4) == ".glb") {
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);
        } else {
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath);
        }
        
        if (!warn.empty()) {
            std::cout << "[GLTFLoader] Warning: " << warn << std::endl;
        }
        
        if (!err.empty()) {
            std::cerr << "[GLTFLoader] Error: " << err << std::endl;
        }
        
        if (!ret) {
            std::cerr << "[GLTFLoader] Failed to load glTF file: " << filepath << std::endl;
            return false;
        }
        
        std::cout << "[GLTFLoader] Loading glTF: " << filepath << std::endl;
        std::cout << "  Meshes: " << model.meshes.size() << std::endl;
        std::cout << "  Materials: " << model.materials.size() << std::endl;
        std::cout << "  Textures: " << model.textures.size() << std::endl;
        std::cout << "  Images: " << model.images.size() << std::endl;
        
        // Extract textures
        extract_textures(model, out_model);
        
        // Extract materials
        extract_materials(model, out_model);
        
        // Extract meshes
        extract_meshes(model, out_model);
        
        out_model.name = filepath;
        
        std::cout << "[GLTFLoader] Loaded " << out_model.primitives.size() 
                  << " primitives from " << filepath << std::endl;
        
        return out_model.is_valid();
    }

private:
    /**
     * Extract textures from glTF model
     */
    static void extract_textures(const tinygltf::Model& model, GLTFModel& out_model) {
        for (size_t i = 0; i < model.images.size(); ++i) {
            const tinygltf::Image& image = model.images[i];
            
            Texture tex;
            tex.width = image.width;
            tex.height = image.height;
            tex.channels = image.component;
            
            // Convert format
            if (image.component == 1) {
                tex.format = TextureFormat::R8;
            } else if (image.component == 2) {
                tex.format = TextureFormat::RG8;
            } else if (image.component == 3) {
                tex.format = TextureFormat::RGB8;
            } else if (image.component == 4) {
                tex.format = TextureFormat::RGBA8;
            }
            
            // Copy pixel data
            tex.data = image.image;
            
            out_model.textures.push_back(tex);
            
            std::cout << "[GLTFLoader]   Texture " << i << ": " 
                      << tex.width << "x" << tex.height 
                      << " (" << tex.channels << " channels)" << std::endl;
        }
    }
    
    /**
     * Extract materials from glTF model
     */
    static void extract_materials(const tinygltf::Model& model, GLTFModel& out_model) {
        for (size_t i = 0; i < model.materials.size(); ++i) {
            const tinygltf::Material& mat = model.materials[i];
            
            GLTFMaterial gltf_mat;
            gltf_mat.name = mat.name;
            
            // PBR metallic-roughness
            if (mat.pbrMetallicRoughness.baseColorFactor.size() >= 4) {
                for (int j = 0; j < 4; ++j) {
                    gltf_mat.base_color_factor[j] = 
                        static_cast<float>(mat.pbrMetallicRoughness.baseColorFactor[j]);
                }
            }
            
            gltf_mat.metallic_factor = 
                static_cast<float>(mat.pbrMetallicRoughness.metallicFactor);
            gltf_mat.roughness_factor = 
                static_cast<float>(mat.pbrMetallicRoughness.roughnessFactor);
            
            // Texture indices
            gltf_mat.base_color_texture_index = 
                mat.pbrMetallicRoughness.baseColorTexture.index;
            gltf_mat.metallic_roughness_texture_index = 
                mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
            gltf_mat.normal_texture_index = mat.normalTexture.index;
            gltf_mat.occlusion_texture_index = mat.occlusionTexture.index;
            gltf_mat.emissive_texture_index = mat.emissiveTexture.index;
            
            // Emissive factor
            if (mat.emissiveFactor.size() >= 3) {
                for (int j = 0; j < 3; ++j) {
                    gltf_mat.emissive_factor[j] = static_cast<float>(mat.emissiveFactor[j]);
                }
            }
            
            // Alpha mode
            if (mat.alphaMode == "OPAQUE") {
                gltf_mat.alpha_mode = GLTFMaterial::AlphaMode::Opaque;
            } else if (mat.alphaMode == "MASK") {
                gltf_mat.alpha_mode = GLTFMaterial::AlphaMode::Mask;
            } else if (mat.alphaMode == "BLEND") {
                gltf_mat.alpha_mode = GLTFMaterial::AlphaMode::Blend;
            }
            
            gltf_mat.alpha_cutoff = static_cast<float>(mat.alphaCutoff);
            gltf_mat.double_sided = mat.doubleSided;
            
            out_model.materials.push_back(gltf_mat);
            
            std::cout << "[GLTFLoader]   Material " << i << ": " << gltf_mat.name << std::endl;
        }
    }
    
    /**
     * Extract meshes from glTF model
     */
    static void extract_meshes(const tinygltf::Model& model, GLTFModel& out_model) {
        for (size_t mesh_idx = 0; mesh_idx < model.meshes.size(); ++mesh_idx) {
            const tinygltf::Mesh& mesh = model.meshes[mesh_idx];
            
            std::cout << "[GLTFLoader]   Mesh " << mesh_idx << ": " << mesh.name 
                      << " (" << mesh.primitives.size() << " primitives)" << std::endl;
            
            // Process each primitive
            for (size_t prim_idx = 0; prim_idx < mesh.primitives.size(); ++prim_idx) {
                const tinygltf::Primitive& primitive = mesh.primitives[prim_idx];
                
                // Only support triangles
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                    std::cout << "[GLTFLoader]     Skipping non-triangle primitive" << std::endl;
                    continue;
                }
                
                GLTFPrimitive gltf_prim;
                gltf_prim.name = mesh.name;
                gltf_prim.material_index = primitive.material;
                
                // Extract vertex data
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;
                
                // Get positions
                std::vector<float> positions;
                if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                    extract_attribute(model, primitive, "POSITION", positions);
                }
                
                // Get normals
                std::vector<float> normals;
                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    extract_attribute(model, primitive, "NORMAL", normals);
                }
                
                // Get texture coordinates
                std::vector<float> texcoords;
                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    extract_attribute(model, primitive, "TEXCOORD_0", texcoords);
                }
                
                // Build vertices
                size_t vertex_count = positions.size() / 3;
                vertices.reserve(vertex_count);
                
                for (size_t i = 0; i < vertex_count; ++i) {
                    Vertex v;
                    
                    // Position
                    v.x = positions[i * 3 + 0];
                    v.y = positions[i * 3 + 1];
                    v.z = positions[i * 3 + 2];
                    
                    // Normal
                    if (i * 3 < normals.size()) {
                        v.nx = normals[i * 3 + 0];
                        v.ny = normals[i * 3 + 1];
                        v.nz = normals[i * 3 + 2];
                    } else {
                        v.nx = 0.0f; v.ny = 1.0f; v.nz = 0.0f;
                    }
                    
                    // Texture coordinates
                    if (i * 2 < texcoords.size()) {
                        v.u = texcoords[i * 2 + 0];
                        v.v = texcoords[i * 2 + 1];
                    } else {
                        v.u = 0.0f; v.v = 0.0f;
                    }
                    
                    vertices.push_back(v);
                }
                
                // Extract indices
                if (primitive.indices >= 0) {
                    extract_indices(model, primitive, indices);
                } else {
                    // No indices, generate them
                    for (size_t i = 0; i < vertex_count; ++i) {
                        indices.push_back(static_cast<uint32_t>(i));
                    }
                }
                
                // Set mesh data
                gltf_prim.mesh.set_vertices(vertices);
                gltf_prim.mesh.set_indices(indices);
                
                out_model.primitives.push_back(gltf_prim);
                
                std::cout << "[GLTFLoader]     Primitive " << prim_idx 
                          << ": " << vertices.size() << " vertices, " 
                          << indices.size() / 3 << " triangles" << std::endl;
            }
        }
    }
    
    /**
     * Extract vertex attribute from glTF accessor
     */
    static void extract_attribute(const tinygltf::Model& model, 
                                  const tinygltf::Primitive& primitive,
                                  const std::string& attr_name,
                                  std::vector<float>& out_data) {
        auto it = primitive.attributes.find(attr_name);
        if (it == primitive.attributes.end()) {
            return;
        }
        
        int accessor_idx = it->second;
        const tinygltf::Accessor& accessor = model.accessors[accessor_idx];
        const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
        
        const uint8_t* data_ptr = buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;
        
        size_t component_count = tinygltf::GetNumComponentsInType(accessor.type);
        size_t stride = accessor.ByteStride(buffer_view);
        
        out_data.reserve(accessor.count * component_count);
        
        for (size_t i = 0; i < accessor.count; ++i) {
            const uint8_t* element_ptr = data_ptr + i * stride;
            
            for (size_t j = 0; j < component_count; ++j) {
                float value = 0.0f;
                
                // Convert based on component type
                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    value = *reinterpret_cast<const float*>(element_ptr + j * sizeof(float));
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    value = static_cast<float>(*reinterpret_cast<const uint16_t*>(element_ptr + j * sizeof(uint16_t)));
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
                    value = static_cast<float>(*reinterpret_cast<const int16_t*>(element_ptr + j * sizeof(int16_t)));
                }
                
                out_data.push_back(value);
            }
        }
    }
    
    /**
     * Extract indices from glTF accessor
     */
    static void extract_indices(const tinygltf::Model& model,
                               const tinygltf::Primitive& primitive,
                               std::vector<uint32_t>& out_indices) {
        const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
        const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
        
        const uint8_t* data_ptr = buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;
        
        out_indices.reserve(accessor.count);
        
        for (size_t i = 0; i < accessor.count; ++i) {
            uint32_t index = 0;
            
            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                index = reinterpret_cast<const uint32_t*>(data_ptr)[i];
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                index = reinterpret_cast<const uint16_t*>(data_ptr)[i];
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                index = data_ptr[i];
            }
            
            out_indices.push_back(index);
        }
    }
};

} // namespace astraeus

#endif // ASTRAEUS_GLTF_LOADER_HPP
