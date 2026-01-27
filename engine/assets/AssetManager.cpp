#include "AssetManager.hpp"
#include "../renderer/RenderDevice.hpp"
#include <iostream>

namespace astraeus {

AssetManager::AssetManager(RenderDevice* device)
    : device_(device)
    , is_initialized_(false)
    , next_handle_(1)
{
}

AssetManager::~AssetManager() {
    shutdown();
}

bool AssetManager::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[AssetManager] Initializing" << std::endl;
    
    is_initialized_ = true;
    return true;
}

void AssetManager::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[AssetManager] Shutting down" << std::endl;
    is_initialized_ = false;
}

uint32_t AssetManager::load_model(const char* path) {
    if (!is_initialized_ || !path) {
        return 0;
    }

    std::cout << "[AssetManager] Loading model: " << path << std::endl;
    
    // TODO: Implement actual model loading
    // Return a handle
    return next_handle_++;
}

uint32_t AssetManager::load_texture(const char* path) {
    if (!is_initialized_ || !path) {
        return 0;
    }

    std::cout << "[AssetManager] Loading texture: " << path << std::endl;
    
    // TODO: Implement actual texture loading
    return next_handle_++;
}

void AssetManager::unload_asset(uint32_t handle) {
    if (handle == 0) {
        return;
    }

    std::cout << "[AssetManager] Unloading asset: " << handle << std::endl;
    
    // TODO: Implement actual unloading
}

} // namespace astraeus
