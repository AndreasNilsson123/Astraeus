#ifndef ASTRAEUS_ASSET_DATABASE_HPP
#define ASTRAEUS_ASSET_DATABASE_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include "../geometry/Mesh.hpp"

namespace astraeus {

/**
 * Asset metadata - tracks information about loaded assets
 */
struct AssetMetadata {
    std::string uri;                    // Original URI (file path or URL)
    std::string hash;                   // Content hash (for cache validation)
    uint64_t size_bytes = 0;            // Asset size in bytes
    uint64_t load_time_ms = 0;          // Time taken to load (milliseconds)
    uint32_t reference_count = 1;       // Number of active references
    bool is_loaded = false;             // Whether asset is fully loaded
    
    AssetMetadata() = default;
    AssetMetadata(const std::string& uri_) : uri(uri_) {}
};

/**
 * Asset load state - tracks async loading progress
 */
enum class AssetLoadState {
    Pending,    // Queued for loading
    Loading,    // Currently being loaded
    Ready,      // Loaded and ready to use
    Error       // Failed to load
};

/**
 * Async load request - stub for future async loading implementation
 */
struct AsyncLoadRequest {
    uint32_t asset_id = 0;
    std::string uri;
    AssetLoadState state = AssetLoadState::Pending;
    std::string error_message;
    
    // Callback stub (not used yet, but defined for future implementation)
    // std::function<void(uint32_t asset_id, bool success)> callback;
};

/**
 * AssetDatabase - Central database for all asset management
 * 
 * Features:
 * - URI-based asset identification with content hashing
 * - Asset metadata tracking
 * - Reference counting for safe unloading
 * - Cache key generation (URI + hash)
 * - Async loading hooks (stubbed for future implementation)
 */
class AssetDatabase {
public:
    AssetDatabase() : next_asset_id_(1) {}
    ~AssetDatabase() = default;

    /**
     * Generate a cache key from URI and optional hash
     * @param uri Asset URI (file path)
     * @param hash Optional content hash (empty if not computed yet)
     * @return Cache key string
     */
    static std::string generate_cache_key(const std::string& uri, const std::string& hash = "") {
        if (hash.empty()) {
            // If no hash provided, use URI as key
            return uri;
        }
        // Combine URI and hash for cache key
        return uri + "#" + hash;
    }

    /**
     * Compute a simple hash of file contents
     * Note: This is a simple hash for demonstration. Production code should use
     * a proper cryptographic hash (SHA-256, etc.)
     * @param filepath Path to file
     * @return Hash string (hex)
     */
    static std::string compute_file_hash(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }

        // Simple hash: combine file size with first and last bytes
        // In production, use a proper hash function
        file.seekg(0, std::ios::end);
        uint64_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        uint64_t hash = size;
        
        if (size > 0) {
            char first_byte, last_byte;
            file.read(&first_byte, 1);
            hash = (hash * 31) + static_cast<uint64_t>(first_byte);
            
            if (size > 1) {
                file.seekg(-1, std::ios::end);
                file.read(&last_byte, 1);
                hash = (hash * 31) + static_cast<uint64_t>(last_byte);
            }
        }

        file.close();

        // Convert to hex string
        char hex[17];
        snprintf(hex, sizeof(hex), "%016lx", hash);
        return std::string(hex);
    }

    /**
     * Register an asset in the database
     * @param uri Asset URI (file path)
     * @param compute_hash Whether to compute content hash
     * @return Asset ID (0 if already exists, reuse existing)
     */
    uint32_t register_asset(const std::string& uri, bool compute_hash = true) {
        // Compute hash if requested
        std::string hash;
        if (compute_hash) {
            hash = compute_file_hash(uri);
        }

        // Generate cache key
        std::string cache_key = generate_cache_key(uri, hash);

        // Check if already registered
        auto it = cache_key_to_id_.find(cache_key);
        if (it != cache_key_to_id_.end()) {
            uint32_t existing_id = it->second;
            // Increment reference count
            auto meta_it = metadata_.find(existing_id);
            if (meta_it != metadata_.end()) {
                meta_it->second.reference_count++;
                std::cout << "[AssetDatabase] Asset already registered: " << uri 
                          << " (id=" << existing_id << ", refs=" 
                          << meta_it->second.reference_count << ")" << std::endl;
            }
            return existing_id;
        }

        // Register new asset
        uint32_t asset_id = next_asset_id_++;
        
        AssetMetadata meta(uri);
        meta.hash = hash;
        meta.reference_count = 1;
        
        metadata_[asset_id] = meta;
        cache_key_to_id_[cache_key] = asset_id;
        uri_to_id_[uri] = asset_id;

        std::cout << "[AssetDatabase] Registered new asset: " << uri 
                  << " (id=" << asset_id << ", hash=" << hash << ")" << std::endl;

        return asset_id;
    }

    /**
     * Mark an asset as loaded
     * @param asset_id Asset ID
     * @param size_bytes Size in bytes
     * @param load_time_ms Load time in milliseconds
     */
    void mark_loaded(uint32_t asset_id, uint64_t size_bytes, uint64_t load_time_ms) {
        auto it = metadata_.find(asset_id);
        if (it != metadata_.end()) {
            it->second.is_loaded = true;
            it->second.size_bytes = size_bytes;
            it->second.load_time_ms = load_time_ms;
            
            std::cout << "[AssetDatabase] Asset loaded: id=" << asset_id 
                      << ", size=" << size_bytes << " bytes"
                      << ", time=" << load_time_ms << "ms" << std::endl;
        }
    }

    /**
     * Release a reference to an asset
     * @param asset_id Asset ID
     * @return true if asset should be deleted (ref count reached 0)
     */
    bool release_reference(uint32_t asset_id) {
        auto it = metadata_.find(asset_id);
        if (it == metadata_.end()) {
            return false;
        }

        if (it->second.reference_count > 0) {
            it->second.reference_count--;
            
            std::cout << "[AssetDatabase] Released reference: id=" << asset_id 
                      << ", refs=" << it->second.reference_count << std::endl;

            if (it->second.reference_count == 0) {
                // Remove from all maps
                std::string uri = it->second.uri;
                std::string cache_key = generate_cache_key(uri, it->second.hash);
                
                cache_key_to_id_.erase(cache_key);
                uri_to_id_.erase(uri);
                metadata_.erase(it);
                
                std::cout << "[AssetDatabase] Asset unregistered: id=" << asset_id << std::endl;
                return true;
            }
        }

        return false;
    }

    /**
     * Get asset metadata
     * @param asset_id Asset ID
     * @return Pointer to metadata, or nullptr if not found
     */
    const AssetMetadata* get_metadata(uint32_t asset_id) const {
        auto it = metadata_.find(asset_id);
        return (it != metadata_.end()) ? &it->second : nullptr;
    }

    /**
     * Check if asset is registered
     * @param uri Asset URI
     * @return Asset ID if registered, 0 otherwise
     */
    uint32_t find_by_uri(const std::string& uri) const {
        auto it = uri_to_id_.find(uri);
        return (it != uri_to_id_.end()) ? it->second : 0;
    }

    /**
     * Get total number of registered assets
     */
    size_t get_asset_count() const {
        return metadata_.size();
    }

    /**
     * Get total memory usage of loaded assets (approximate)
     */
    uint64_t get_total_size_bytes() const {
        uint64_t total = 0;
        for (const auto& pair : metadata_) {
            total += pair.second.size_bytes;
        }
        return total;
    }

    /**
     * Clear all assets (for shutdown)
     */
    void clear() {
        metadata_.clear();
        cache_key_to_id_.clear();
        uri_to_id_.clear();
        next_asset_id_ = 1;
        
        std::cout << "[AssetDatabase] Cleared all assets" << std::endl;
    }

    /**
     * Create an async load request (stub for future implementation)
     * @param uri Asset URI
     * @return Load request with pending state
     */
    AsyncLoadRequest create_async_request(const std::string& uri) {
        AsyncLoadRequest request;
        request.uri = uri;
        request.state = AssetLoadState::Pending;
        
        // In a full implementation, this would:
        // 1. Queue the request for a background thread
        // 2. Return immediately
        // 3. Update state as loading progresses
        // 4. Call callback when complete
        
        std::cout << "[AssetDatabase] Async load request created (stub): " << uri << std::endl;
        
        return request;
    }

private:
    uint32_t next_asset_id_;
    
    // Asset metadata indexed by ID
    std::unordered_map<uint32_t, AssetMetadata> metadata_;
    
    // Cache key (URI+hash) to asset ID mapping
    std::unordered_map<std::string, uint32_t> cache_key_to_id_;
    
    // URI to asset ID mapping (for quick lookup)
    std::unordered_map<std::string, uint32_t> uri_to_id_;
};

} // namespace astraeus

#endif // ASTRAEUS_ASSET_DATABASE_HPP
