#ifndef ASTRAEUS_MESH_HPP
#define ASTRAEUS_MESH_HPP

#include <cstdint>
#include <vector>

namespace astraeus {

/**
 * Vertex data structure.
 */
struct Vertex {
    float x, y, z;       // Position
    float nx, ny, nz;    // Normal
    float u, v;          // UV coordinates
};

/**
 * Mesh represents geometry data.
 */
class Mesh {
public:
    Mesh() = default;
    ~Mesh() = default;

    inline void set_vertices(const std::vector<Vertex>& vertices) {
        vertices_ = vertices;
    }

    inline void set_indices(const std::vector<uint32_t>& indices) {
        indices_ = indices;
    }

    const std::vector<Vertex>& get_vertices() const { return vertices_; }
    const std::vector<uint32_t>& get_indices() const { return indices_; }

    uint32_t get_vertex_count() const { return static_cast<uint32_t>(vertices_.size()); }
    uint32_t get_index_count() const { return static_cast<uint32_t>(indices_.size()); }

private:
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;
};

} // namespace astraeus

#endif // ASTRAEUS_MESH_HPP
