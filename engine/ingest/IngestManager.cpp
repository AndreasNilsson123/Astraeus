#include "IngestManager.hpp"
#include "../scene/World.hpp"
#include <iostream>

namespace astraeus {

IngestManager::IngestManager(World* world)
    : world_(world)
    , is_initialized_(false)
{
}

IngestManager::~IngestManager() {
    shutdown();
}

bool IngestManager::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[IngestManager] Initializing" << std::endl;
    
    is_initialized_ = true;
    return true;
}

void IngestManager::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[IngestManager] Shutting down" << std::endl;
    is_initialized_ = false;
}

bool IngestManager::ingest(const void* data, uint32_t size, uint32_t format) {
    if (!is_initialized_ || !data || size == 0) {
        return false;
    }

    std::cout << "[IngestManager] Ingesting " << size << " bytes of format " << format << std::endl;
    
    // TODO: Parse and convert external data to scene entities
    // This is where external physics/simulation data gets translated
    
    return true;
}

} // namespace astraeus
