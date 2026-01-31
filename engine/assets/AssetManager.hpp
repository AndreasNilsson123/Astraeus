#ifndef ASTRAEUS_ASSET_MANAGER_HPP
#define ASTRAEUS_ASSET_MANAGER_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include "MeshLoader.hpp"
#include "GPUUploadQueue.hpp"
#include "AssetDatabase.hpp"
#include "GLTFLoader.hpp"
#include "../geometry/Mesh.hpp"

namespace astraeus {

class RenderDevice;

/**
 * AssetManager handles loading and managing models, textures, and GPU upload.
 * Enhanced with AssetDatabase for URI+hash caching and glTF support.
 */
class AssetManager {
public:
    inline explicit AssetManager(RenderDevice* device)
        : device_(device)
        , is_initialized_(false)
        , next_handle_(1)
        , asset_database_()
        , gpu_upload_queue_()

    {
    }

    inline ~AssetManager() {
        shutdown();
    }

    inline bool initialize() {
        if (is_initialized_) {
            return true;
        }
        
        // Validate we have a valid render device
        if (!device_) {
            std::cerr << "[AssetManager] No render device provided" << std::endl;
            return false;
        }

        std::cout << "[AssetManager] Initializing" << std::endl;
        
        // Initialize GPU upload queue
        if (!gpu_upload_queue_.initialize()) {
            std::cerr << "[AssetManager] Failed to initialize GPU upload queue" << std::endl;
            return false;
        }
        
        is_initialized_ = true;
        return true;
    }

    inline void shutdown() {
        if (!is_initialized_) {
            return;
        }

        std::cout << "[AssetManager] Shutting down" << std::endl;
        
        // Unload all assets
        for (auto& pair : asset_cache_) {
            gpu_upload_queue_.release(pair.first);
        }
        asset_cache_.clear();
        path_to_id_.clear();
        
        // Clear asset database
        asset_database_.clear();
        
        gpu_upload_queue_.shutdown();
        is_initialized_ = false;
    }

    /**
     * Load a model asset from OBJ or glTF file.
     * @param path File path to model (supports .obj, .gltf, .glb files)
     * @return Asset handle or 0 on failure
     */
    inline uint32_t load_model(const char* path) {
        if (!is_initialized_ || !path) {
            return 0;
        }

        std::string path_str(path);
        
        // Register asset with database (uses URI+hash for caching)
        auto start_time = std::chrono::high_resolution_clock::now();
        uint32_t asset_id = asset_database_.register_asset(path_str, true);
        
        // Check if already loaded
        auto it = path_to_id_.find(path_str);
        if (it != path_to_id_.end() && it->second == asset_id) {
            // Already loaded, just return the ID
            auto mesh_it = asset_cache_.find(asset_id);
            if (mesh_it != asset_cache_.end()) {
                gpu_upload_queue_.enqueue_upload(asset_id, mesh_it->second);
                std::cout << "[AssetManager] Model already loaded: " << path 
                          << " (asset_id=" << asset_id << ")" << std::endl;
                return asset_id;
            }
        }

        std::cout << "[AssetManager] Loading model: " << path << std::endl;
        
        // Determine file type
        bool is_gltf = (path_str.substr(path_str.length() - 5) == ".gltf" ||
                       path_str.substr(path_str.length() - 4) == ".glb");
        
        Mesh mesh;
        bool loaded = false;
        
        if (is_gltf) {
            // Load glTF file
            GLTFModel gltf_model;
            if (!GLTFLoader::load_gltf(path_str, gltf_model)) {
                std::cerr << "[AssetManager] Failed to load glTF model: " << path << std::endl;
                return 0;
            }
            
            // For now, use only the first primitive
            // TODO: Support multiple primitives per model
            if (!gltf_model.primitives.empty()) {
                mesh = gltf_model.primitives[0].mesh;
                loaded = true;
                
                std::cout << "[AssetManager] Loaded glTF with " 
                          << gltf_model.primitives.size() << " primitives, "
                          << gltf_model.materials.size() << " materials, "
                          << gltf_model.textures.size() << " textures" << std::endl;
            }
        } else {
            // Load OBJ file
            loaded = MeshLoader::load_obj(path_str, mesh);
        }
        
        if (!loaded) {
            std::cerr << "[AssetManager] Failed to load model: " << path << std::endl;
            return 0;
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto load_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
        
        // Calculate approximate size
        uint64_t size_bytes = mesh.get_vertex_data().size() * sizeof(float) + 
                             mesh.get_indices().size() * sizeof(uint32_t);
        
        // Mark as loaded in database
        asset_database_.mark_loaded(asset_id, size_bytes, load_time_ms);
        
        // Cache mesh data
        asset_cache_[asset_id] = mesh;
        path_to_id_[path_str] = asset_id;
        
        // Enqueue for GPU upload
        gpu_upload_queue_.enqueue_upload(asset_id, mesh);
        
        std::cout << "[AssetManager] Model loaded successfully: " << path 
                  << " (asset_id=" << asset_id << ")" << std::endl;
        
        return asset_id;
    }

    /**
     * Load a texture asset.
     * @param path File path to texture
     * @return Asset handle or 0 on failure
     */
    inline uint32_t load_texture(const char* path) {
        if (!is_initialized_ || !path) {
            return 0;
        }

        std::cout << "[AssetManager] Loading texture: " << path << std::endl;
        
        // TODO: Implement actual texture loading
        return next_handle_++;
    }

    /**
     * Unload an asset.
     * Decrements reference count and deletes GPU resources if ref count reaches 0.
     * @param handle Asset handle
     */
    inline void unload_asset(uint32_t handle) {
        if (handle == 0) {
            return;
        }

        std::cout << "[AssetManager] Unloading asset: " << handle << std::endl;
        
        // Release reference in database
        bool should_delete = asset_database_.release_reference(handle);
        
        // Release GPU resources (ref counted)
        bool gpu_deleted = gpu_upload_queue_.release(handle);
        
        if (should_delete || gpu_deleted) {
            // Remove from cache if resources were deleted
            asset_cache_.erase(handle);
            
            // Remove from path mapping
            for (auto it = path_to_id_.begin(); it != path_to_id_.end(); ) {
                if (it->second == handle) {
                    it = path_to_id_.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    
    /**
     * Process pending GPU uploads.
     * Should be called once per frame on the render thread.
     */
    inline void process_uploads() {
        gpu_upload_queue_.process_uploads(1); // Process 1 upload per frame
    }
    
    /**
     * Get GPU mesh for rendering.
     * @param asset_id Asset ID
     * @return Pointer to GPU mesh, or nullptr if not uploaded
     */
    inline const GPUMesh* get_gpu_mesh(uint32_t asset_id) const {
        return gpu_upload_queue_.get_gpu_mesh(asset_id);
    }
    
    /**
     * Check if asset is ready for rendering (uploaded to GPU).
     */
    inline bool is_ready(uint32_t asset_id) const {
        return gpu_upload_queue_.is_uploaded(asset_id);
    }
    
    /**
     * Get asset database (for advanced queries)
     */
    const AssetDatabase& get_database() const {
        return asset_database_;
    }
    
    /**
     * Get asset metadata
     * @param asset_id Asset ID
     * @return Pointer to metadata, or nullptr if not found
     */
    const AssetMetadata* get_asset_metadata(uint32_t asset_id) const {
        return asset_database_.get_metadata(asset_id);
    }
    
    /**
     * Get total number of loaded assets
     */
    size_t get_asset_count() const {
        return asset_database_.get_asset_count();
    }
    
    /**
     * Get total memory usage of assets (approximate)
     */
    uint64_t get_total_memory_usage() const {
        return asset_database_.get_total_size_bytes();
    }

private:
    RenderDevice* device_;
    bool is_initialized_;
    uint32_t next_handle_;
    
    // Asset database (URI+hash based caching)
    AssetDatabase asset_database_;
    
    // Asset cache (CPU-side mesh data)
    std::unordered_map<uint32_t, Mesh> asset_cache_;
    
    // Path to asset ID mapping (for deduplication)
    std::unordered_map<std::string, uint32_t> path_to_id_;
    
    // GPU upload queue
    GPUUploadQueue gpu_upload_queue_;
};

} // namespace astraeus

#endif // ASTRAEUS_ASSET_MANAGER_HPP
