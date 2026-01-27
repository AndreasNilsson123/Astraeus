#include "Mesh.hpp"

namespace astraeus {

void Mesh::set_vertices(const std::vector<Vertex>& vertices) {
    vertices_ = vertices;
}

void Mesh::set_indices(const std::vector<uint32_t>& indices) {
    indices_ = indices;
}

} // namespace astraeus
