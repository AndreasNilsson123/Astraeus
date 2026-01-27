#ifndef ASTRAEUS_ASSET_MANAGER_HPP
#define ASTRAEUS_ASSET_MANAGER_HPP

#include <cstdint>

namespace astraeus {

class RenderDevice;

/**
 * AssetManager handles loading and managing models, textures, and GPU upload.
 */
class AssetManager {
public:
    explicit AssetManager(RenderDevice* device);
    ~AssetManager();

    bool initialize();
    void shutdown();

    /**
     * Load a model asset.
     * @param path File path to model
     * @return Asset handle or 0 on failure
     */
    uint32_t load_model(const char* path);

    /**
     * Load a texture asset.
     * @param path File path to texture
     * @return Asset handle or 0 on failure
     */
    uint32_t load_texture(const char* path);

    /**
     * Unload an asset.
     * @param handle Asset handle
     */
    void unload_asset(uint32_t handle);

private:
    RenderDevice* device_;
    bool is_initialized_;
    uint32_t next_handle_;
};

} // namespace astraeus

#endif // ASTRAEUS_ASSET_MANAGER_HPP
