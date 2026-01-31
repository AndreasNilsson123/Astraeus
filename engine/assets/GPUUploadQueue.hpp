#ifndef ASTRAEUS_GPU_UPLOAD_QUEUE_HPP
#define ASTRAEUS_GPU_UPLOAD_QUEUE_HPP

#include <cstdint>
#include <vector>
#include <queue>
#include <unordered_map>
#include <iostream>
#include "GPUMesh.hpp"
#include "../geometry/Mesh.hpp"
#include "platform/GL/GLHeaders.hpp"

namespace astraeus {

/**
 * GPUUploadQueue manages uploading mesh data to the GPU with staging buffers.
 * Handles deferred uploads and resource management.
 */
class GPUUploadQueue {
public:
    inline GPUUploadQueue() : is_initialized_(false) {}
    
    inline ~GPUUploadQueue() {
        shutdown();
    }

    inline bool initialize() {
        if (is_initialized_) {
            return true;
        }

        std::cout << "[GPUUploadQueue] Initializing GPU upload queue" << std::endl;
        is_initialized_ = true;
        return true;
    }

    inline void shutdown() {
        if (!is_initialized_) {
            return;
        }

        std::cout << "[GPUUploadQueue] Shutting down" << std::endl;

        // Clear pending uploads
        while (!upload_queue_.empty()) {
            upload_queue_.pop();
        }

        // Delete all GPU resources
        for (auto& pair : gpu_meshes_) {
            delete_gpu_mesh(pair.second);
        }
        gpu_meshes_.clear();

        is_initialized_ = false;
    }

    /**
     * Enqueue a mesh for GPU upload.
     * @param asset_id Unique asset ID
     * @param mesh Mesh to upload
     */
    inline void enqueue_upload(uint32_t asset_id, const Mesh& mesh) {
        if (!is_initialized_) {
            std::cerr << "[GPUUploadQueue] Not initialized" << std::endl;
            return;
        }

        GPUUploadRequest request;
        request.asset_id = asset_id;
        
        // Convert mesh to interleaved vertex data
        const auto& vertices = mesh.get_vertices();
        request.vertex_count = static_cast<uint32_t>(vertices.size());
        request.vertex_stride = sizeof(Vertex);
        
        // Interleave vertex data
        request.vertex_data.reserve(vertices.size() * (sizeof(Vertex) / sizeof(float)));
        for (const auto& v : vertices) {
            request.vertex_data.push_back(v.x);
            request.vertex_data.push_back(v.y);
            request.vertex_data.push_back(v.z);
            request.vertex_data.push_back(v.nx);
            request.vertex_data.push_back(v.ny);
            request.vertex_data.push_back(v.nz);
            request.vertex_data.push_back(v.u);
            request.vertex_data.push_back(v.v);
        }

        // Copy index data
        request.index_data = mesh.get_indices();
        request.index_count = static_cast<uint32_t>(request.index_data.size());

        // Log before moving
        std::cout << "[GPUUploadQueue] Enqueued upload for asset " << asset_id 
                  << " (" << request.vertex_count << " vertices, " 
                  << request.index_count / 3 << " triangles)" << std::endl;

        upload_queue_.push(std::move(request));
    }

    /**
     * Process pending uploads (should be called on render thread).
     * @param max_uploads_per_frame Maximum number of uploads to process per frame
     */
    inline void process_uploads(uint32_t max_uploads_per_frame = 1) {
        if (!is_initialized_) {
            return;
        }

        uint32_t processed = 0;
        while (!upload_queue_.empty() && processed < max_uploads_per_frame) {
            GPUUploadRequest& request = upload_queue_.front();
            
            // Upload to GPU
            GPUMesh gpu_mesh;
            if (upload_to_gpu(request, gpu_mesh)) {
                // Store GPU mesh
                gpu_meshes_[request.asset_id] = gpu_mesh;
                std::cout << "[GPUUploadQueue] Uploaded asset " << request.asset_id 
                          << " to GPU (VAO=" << gpu_mesh.vao << ")" << std::endl;
            } else {
                std::cerr << "[GPUUploadQueue] Failed to upload asset " << request.asset_id << std::endl;
            }

            // Check if already uploaded
            if (gpu_meshes_.find(request.asset_id) != gpu_meshes_.end()) {
                // Already uploaded, just increment ref count
                gpu_meshes_[request.asset_id].ref_count++;
                std::cout << "[GPUUploadQueue] Asset " << request.asset_id
                          << " already uploaded, ref_count=" << gpu_meshes_[request.asset_id].ref_count << std::endl;
                return;
            }

            upload_queue_.pop();
            processed++;
        }
    }

    /**
     * Get GPU mesh for an asset.
     * @param asset_id Asset ID
     * @return Pointer to GPU mesh, or nullptr if not found
     */
    inline const GPUMesh* get_gpu_mesh(uint32_t asset_id) const {
        auto it = gpu_meshes_.find(asset_id);
        if (it != gpu_meshes_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    /**
     * Check if an asset is uploaded to GPU.
     */
    inline bool is_uploaded(uint32_t asset_id) const {
        return gpu_meshes_.find(asset_id) != gpu_meshes_.end();
    }

    /**
     * Release a reference to an asset. Deletes GPU resources if ref count reaches 0.
     * @param asset_id Asset ID to release
     * @return true if resource was deleted, false otherwise
     */
    inline bool release(uint32_t asset_id) {
        auto it = gpu_meshes_.find(asset_id);
        if (it == gpu_meshes_.end()) {
            // Asset not found - might be in upload queue or already deleted
            return false;
        }

        // Prevent underflow
        if (it->second.ref_count == 0) {
            std::cerr << "[GPUUploadQueue] Warning: ref_count already 0 for asset " << asset_id << std::endl;
            return false;
        }

        it->second.ref_count--;
        std::cout << "[GPUUploadQueue] Released asset " << asset_id 
                  << ", ref_count=" << it->second.ref_count << std::endl;

        if (it->second.ref_count == 0) {
            // Delete GPU resources
            delete_gpu_mesh(it->second);
            gpu_meshes_.erase(it);
            std::cout << "[GPUUploadQueue] Deleted GPU resources for asset " << asset_id << std::endl;
            return true;
        }

        return false;
    }

    /**
     * Get number of pending uploads.
     */
    inline size_t get_pending_count() const {
        return upload_queue_.size();
    }

private:
    /**
     * Upload mesh data to GPU.
     */
    inline bool upload_to_gpu(const GPUUploadRequest& request, GPUMesh& out_mesh) {
        // Generate VAO
        glGenVertexArrays(1, &out_mesh.vao);
        if (out_mesh.vao == 0) {
            std::cerr << "[GPUUploadQueue] Failed to create VAO" << std::endl;
            return false;
        }

        glBindVertexArray(out_mesh.vao);

        // Generate and upload VBO
        glGenBuffers(1, &out_mesh.vbo);
        if (out_mesh.vbo == 0) {
            std::cerr << "[GPUUploadQueue] Failed to create VBO" << std::endl;
            glDeleteVertexArrays(1, &out_mesh.vao);
            return false;
        }

        glBindBuffer(GL_ARRAY_BUFFER, out_mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, 
                     request.vertex_data.size() * sizeof(float),
                     request.vertex_data.data(), 
                     GL_STATIC_DRAW);

        // Set vertex attributes (position, normal, texcoord)
        // Position (x, y, z) - location 0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Normal (nx, ny, nz) - location 1
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

        // TexCoord (u, v) - location 2
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));

        // Generate and upload IBO if indices exist
        if (!request.index_data.empty()) {
            glGenBuffers(1, &out_mesh.ibo);
            if (out_mesh.ibo == 0) {
                std::cerr << "[GPUUploadQueue] Failed to create IBO" << std::endl;
                glDeleteBuffers(1, &out_mesh.vbo);
                glDeleteVertexArrays(1, &out_mesh.vao);
                return false;
            }

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_mesh.ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         request.index_data.size() * sizeof(uint32_t),
                         request.index_data.data(),
                         GL_STATIC_DRAW);
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        out_mesh.vertex_count = request.vertex_count;
        out_mesh.index_count = request.index_count;

        return true;
    }

    /**
     * Delete GPU mesh resources.
     */
    inline void delete_gpu_mesh(GPUMesh& mesh) {
        if (mesh.vao != 0) {
            glDeleteVertexArrays(1, &mesh.vao);
        }
        if (mesh.vbo != 0) {
            glDeleteBuffers(1, &mesh.vbo);
        }
        if (mesh.ibo != 0) {
            glDeleteBuffers(1, &mesh.ibo);
        }
        mesh.invalidate();
    }

    bool is_initialized_;
    std::queue<GPUUploadRequest> upload_queue_;
    std::unordered_map<uint32_t, GPUMesh> gpu_meshes_;
};

} // namespace astraeus

#endif // ASTRAEUS_GPU_UPLOAD_QUEUE_HPP
