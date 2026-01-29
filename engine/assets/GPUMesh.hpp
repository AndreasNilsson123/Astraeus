#ifndef ASTRAEUS_GPU_MESH_HPP
#define ASTRAEUS_GPU_MESH_HPP

#include <cstdint>
#include <vector>

namespace astraeus {

/**
 * GPU mesh resource - represents a mesh uploaded to the GPU.
 * Contains OpenGL handles for VAO, VBO, and IBO.
 */
struct GPUMesh {
    uint32_t vao = 0;           // Vertex Array Object
    uint32_t vbo = 0;           // Vertex Buffer Object
    uint32_t ibo = 0;           // Index Buffer Object
    uint32_t vertex_count = 0;  // Number of vertices
    uint32_t index_count = 0;   // Number of indices
    uint32_t ref_count = 1;     // Reference count for sharing
    
    bool is_valid() const {
        return vao != 0 && vbo != 0;
    }
    
    void invalidate() {
        vao = vbo = ibo = 0;
        vertex_count = index_count = 0;
    }
};

/**
 * GPU upload request - describes a pending mesh upload to the GPU.
 */
struct GPUUploadRequest {
    uint32_t asset_id;                     // Asset ID this upload is for
    std::vector<float> vertex_data;        // Interleaved vertex data
    std::vector<uint32_t> index_data;      // Index data
    uint32_t vertex_stride;                // Stride of vertex data in bytes
    uint32_t vertex_count;                 // Number of vertices
    uint32_t index_count;                  // Number of indices
};

} // namespace astraeus

#endif // ASTRAEUS_GPU_MESH_HPP
