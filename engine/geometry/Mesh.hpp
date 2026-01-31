#ifndef ASTRAEUS_MESH_HPP
#define ASTRAEUS_MESH_HPP

#include <cstdint>
#include <vector>
#include "VertexFormat.hpp"

namespace astraeus {

/**
 * Vertex data structure (legacy/default format).
 * Alias for Vertex_P3N3T2.
 */
using Vertex = Vertex_P3N3T2;

/**
 * Mesh represents geometry data.
 * Stores raw vertex data as floats and provides format metadata.
 */
class Mesh {
public:
    Mesh() 
        : vertex_format_(StandardFormats::create_P3N3T2())
    {}
    
    ~Mesh() = default;

    /**
     * Set vertices using the legacy Vertex structure (P3N3T2)
     */
    inline void set_vertices(const std::vector<Vertex>& vertices) {
        vertices_.clear();
        vertices_.reserve(vertices.size() * 8); // 8 floats per vertex
        
        for (const auto& v : vertices) {
            vertices_.push_back(v.x);
            vertices_.push_back(v.y);
            vertices_.push_back(v.z);
            vertices_.push_back(v.nx);
            vertices_.push_back(v.ny);
            vertices_.push_back(v.nz);
            vertices_.push_back(v.u);
            vertices_.push_back(v.v);
        }
        
        vertex_format_ = StandardFormats::create_P3N3T2();
    }
    
    /**
     * Set raw vertex data with a custom format
     */
    inline void set_vertex_data(const std::vector<float>& data, const VertexFormat& format) {
        vertices_ = data;
        vertex_format_ = format;
    }

    inline void set_indices(const std::vector<uint32_t>& indices) {
        indices_ = indices;
    }
    
    /**
     * Get vertices in legacy format (only works if format is P3N3T2)
     */
    const std::vector<Vertex>& get_vertices() const {
        // This is unsafe if format changed, but kept for backward compatibility
        // In a real implementation, we'd return by value or throw if format doesn't match
        static std::vector<Vertex> legacy_vertices;
        legacy_vertices.clear();
        
        if (vertex_format_.get_stride() == sizeof(Vertex)) {
            const float* data = vertices_.data();
            size_t vertex_count = vertices_.size() / 8;
            legacy_vertices.reserve(vertex_count);
            
            for (size_t i = 0; i < vertex_count; ++i) {
                Vertex v;
                v.x = data[i * 8 + 0];
                v.y = data[i * 8 + 1];
                v.z = data[i * 8 + 2];
                v.nx = data[i * 8 + 3];
                v.ny = data[i * 8 + 4];
                v.nz = data[i * 8 + 5];
                v.u = data[i * 8 + 6];
                v.v = data[i * 8 + 7];
                legacy_vertices.push_back(v);
            }
        }
        
        return legacy_vertices;
    }
    
    /**
     * Get raw vertex data
     */
    const std::vector<float>& get_vertex_data() const { 
        return vertices_; 
    }
    
    const std::vector<uint32_t>& get_indices() const { 
        return indices_; 
    }
    
    const VertexFormat& get_vertex_format() const {
        return vertex_format_;
    }

    uint32_t get_vertex_count() const { 
        if (vertex_format_.get_stride() == 0) return 0;
        return static_cast<uint32_t>(vertices_.size() * sizeof(float) / vertex_format_.get_stride()); 
    }
    
    uint32_t get_index_count() const { 
        return static_cast<uint32_t>(indices_.size()); 
    }

private:
    std::vector<float> vertices_;        // Raw interleaved vertex data
    std::vector<uint32_t> indices_;
    VertexFormat vertex_format_;         // Format descriptor
};

} // namespace astraeus

#endif // ASTRAEUS_MESH_HPP
