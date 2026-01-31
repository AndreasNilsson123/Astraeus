#ifndef ASTRAEUS_MESH_UPLOADER_HPP
#define ASTRAEUS_MESH_UPLOADER_HPP

#include "../geometry/Mesh.hpp"
#include "../geometry/VertexFormat.hpp"
#include <cstdint>
#include <vector>

namespace astraeus {

/**
 * MeshUploader - Utility class for creating and uploading meshes with different formats
 */
class MeshUploader {
public:
    /**
     * Create a mesh with P3N3T2 format (Position + Normal + TexCoord)
     */
    static inline Mesh create_P3N3T2(
        const std::vector<Vertex_P3N3T2>& vertices,
        const std::vector<uint32_t>& indices = {})
    {
        Mesh mesh;
        
        // Convert to raw float data
        const size_t floats_per_vertex = sizeof(Vertex_P3N3T2) / sizeof(float);
        std::vector<float> vertex_data;
        vertex_data.reserve(vertices.size() * floats_per_vertex);
        
        for (const auto& v : vertices) {
            vertex_data.push_back(v.x);
            vertex_data.push_back(v.y);
            vertex_data.push_back(v.z);
            vertex_data.push_back(v.nx);
            vertex_data.push_back(v.ny);
            vertex_data.push_back(v.nz);
            vertex_data.push_back(v.u);
            vertex_data.push_back(v.v);
        }
        
        mesh.set_vertex_data(vertex_data, StandardFormats::create_P3N3T2());
        mesh.set_indices(indices);
        
        return mesh;
    }
    
    /**
     * Create a mesh with P3N3T2C4 format (Position + Normal + TexCoord + Color)
     */
    static inline Mesh create_P3N3T2C4(
        const std::vector<Vertex_P3N3T2C4>& vertices,
        const std::vector<uint32_t>& indices = {})
    {
        Mesh mesh;
        
        // Convert to raw float data
        const size_t floats_per_vertex = sizeof(Vertex_P3N3T2C4) / sizeof(float);
        std::vector<float> vertex_data;
        vertex_data.reserve(vertices.size() * floats_per_vertex);
        
        for (const auto& v : vertices) {
            vertex_data.push_back(v.x);
            vertex_data.push_back(v.y);
            vertex_data.push_back(v.z);
            vertex_data.push_back(v.nx);
            vertex_data.push_back(v.ny);
            vertex_data.push_back(v.nz);
            vertex_data.push_back(v.u);
            vertex_data.push_back(v.v);
            vertex_data.push_back(v.r);
            vertex_data.push_back(v.g);
            vertex_data.push_back(v.b);
            vertex_data.push_back(v.a);
        }
        
        mesh.set_vertex_data(vertex_data, StandardFormats::create_P3N3T2C4());
        mesh.set_indices(indices);
        
        return mesh;
    }
    
    /**
     * Create a mesh with P3C4 format (Position + Color)
     */
    static inline Mesh create_P3C4(
        const std::vector<Vertex_P3C4>& vertices,
        const std::vector<uint32_t>& indices = {})
    {
        Mesh mesh;
        
        // Convert to raw float data
        const size_t floats_per_vertex = sizeof(Vertex_P3C4) / sizeof(float);
        std::vector<float> vertex_data;
        vertex_data.reserve(vertices.size() * floats_per_vertex);
        
        for (const auto& v : vertices) {
            vertex_data.push_back(v.x);
            vertex_data.push_back(v.y);
            vertex_data.push_back(v.z);
            vertex_data.push_back(v.r);
            vertex_data.push_back(v.g);
            vertex_data.push_back(v.b);
            vertex_data.push_back(v.a);
        }
        
        mesh.set_vertex_data(vertex_data, StandardFormats::create_P3C4());
        mesh.set_indices(indices);
        
        return mesh;
    }
    
    /**
     * Create a mesh with P3N3T2TB3 format (Position + Normal + TexCoord + Tangent + Bitangent)
     */
    static inline Mesh create_P3N3T2TB3(
        const std::vector<Vertex_P3N3T2TB3>& vertices,
        const std::vector<uint32_t>& indices = {})
    {
        Mesh mesh;
        
        // Convert to raw float data
        const size_t floats_per_vertex = sizeof(Vertex_P3N3T2TB3) / sizeof(float);
        std::vector<float> vertex_data;
        vertex_data.reserve(vertices.size() * floats_per_vertex);
        
        for (const auto& v : vertices) {
            vertex_data.push_back(v.x);
            vertex_data.push_back(v.y);
            vertex_data.push_back(v.z);
            vertex_data.push_back(v.nx);
            vertex_data.push_back(v.ny);
            vertex_data.push_back(v.nz);
            vertex_data.push_back(v.u);
            vertex_data.push_back(v.v);
            vertex_data.push_back(v.tx);
            vertex_data.push_back(v.ty);
            vertex_data.push_back(v.tz);
            vertex_data.push_back(v.bx);
            vertex_data.push_back(v.by);
            vertex_data.push_back(v.bz);
        }
        
        mesh.set_vertex_data(vertex_data, StandardFormats::create_P3N3T2TB3());
        mesh.set_indices(indices);
        
        return mesh;
    }
    
    /**
     * Create a simple cube mesh with P3N3T2 format
     * Useful for testing
     */
    static inline Mesh create_cube(float size = 1.0f) {
        float half = size * 0.5f;
        
        std::vector<Vertex_P3N3T2> vertices = {
            // Front face (z+)
            {-half, -half,  half,  0, 0, 1,  0, 0},
            { half, -half,  half,  0, 0, 1,  1, 0},
            { half,  half,  half,  0, 0, 1,  1, 1},
            {-half,  half,  half,  0, 0, 1,  0, 1},
            
            // Back face (z-)
            { half, -half, -half,  0, 0, -1,  0, 0},
            {-half, -half, -half,  0, 0, -1,  1, 0},
            {-half,  half, -half,  0, 0, -1,  1, 1},
            { half,  half, -half,  0, 0, -1,  0, 1},
            
            // Right face (x+)
            { half, -half,  half,  1, 0, 0,  0, 0},
            { half, -half, -half,  1, 0, 0,  1, 0},
            { half,  half, -half,  1, 0, 0,  1, 1},
            { half,  half,  half,  1, 0, 0,  0, 1},
            
            // Left face (x-)
            {-half, -half, -half,  -1, 0, 0,  0, 0},
            {-half, -half,  half,  -1, 0, 0,  1, 0},
            {-half,  half,  half,  -1, 0, 0,  1, 1},
            {-half,  half, -half,  -1, 0, 0,  0, 1},
            
            // Top face (y+)
            {-half,  half,  half,  0, 1, 0,  0, 0},
            { half,  half,  half,  0, 1, 0,  1, 0},
            { half,  half, -half,  0, 1, 0,  1, 1},
            {-half,  half, -half,  0, 1, 0,  0, 1},
            
            // Bottom face (y-)
            {-half, -half, -half,  0, -1, 0,  0, 0},
            { half, -half, -half,  0, -1, 0,  1, 0},
            { half, -half,  half,  0, -1, 0,  1, 1},
            {-half, -half,  half,  0, -1, 0,  0, 1},
        };
        
        std::vector<uint32_t> indices = {
            0, 1, 2,  0, 2, 3,    // Front
            4, 5, 6,  4, 6, 7,    // Back
            8, 9, 10, 8, 10, 11,  // Right
            12, 13, 14, 12, 14, 15, // Left
            16, 17, 18, 16, 18, 19, // Top
            20, 21, 22, 20, 22, 23  // Bottom
        };
        
        return create_P3N3T2(vertices, indices);
    }
    
    /**
     * Create a simple triangle mesh with P3C4 format
     * Useful for testing colored vertices
     */
    static inline Mesh create_colored_triangle() {
        std::vector<Vertex_P3C4> vertices = {
            { 0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f}, // Top (red)
            {-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f}, // Bottom-left (green)
            { 0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f}  // Bottom-right (blue)
        };
        
        std::vector<uint32_t> indices = {0, 1, 2};
        
        return create_P3C4(vertices, indices);
    }
};

} // namespace astraeus

#endif // ASTRAEUS_MESH_UPLOADER_HPP
