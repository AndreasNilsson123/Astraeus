#ifndef ASTRAEUS_INGEST_MANAGER_HPP
#define ASTRAEUS_INGEST_MANAGER_HPP

#include <cstdint>
#include <iostream>

namespace astraeus {

class World;

/**
 * IngestManager handles external data ingestion from physics/simulation systems.
 * Converts external data formats into engine scene representation.
 */
class IngestManager {
public:
    explicit IngestManager(World* world);
    ~IngestManager();

    bool initialize();
    void shutdown();

    /**
     * Ingest external simulation data.
     * @param data Pointer to data
     * @param size Size in bytes
     * @param format Format identifier (0=custom binary, 1=JSON, etc.)
     * @return true on success
     */
    bool ingest(const void* data, uint32_t size, uint32_t format);

private:
    World* world_;
    bool is_initialized_;
};

// Inline implementations

inline IngestManager::IngestManager(World* world)
    : world_(world)
    , is_initialized_(false)
{
}

inline IngestManager::~IngestManager() {
    shutdown();
}

inline bool IngestManager::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[IngestManager] Initializing" << std::endl;
    
    is_initialized_ = true;
    return true;
}

inline void IngestManager::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[IngestManager] Shutting down" << std::endl;
    is_initialized_ = false;
}

inline bool IngestManager::ingest(const void* data, uint32_t size, uint32_t format) {
    if (!is_initialized_ || !data || size == 0) {
        return false;
    }

    std::cout << "[IngestManager] Ingesting " << size << " bytes of format " << format << std::endl;
    
    // TODO: Parse and convert external data to scene entities
    // This is where external physics/simulation data gets translated
    
    return true;
}

} // namespace astraeus

#endif // ASTRAEUS_INGEST_MANAGER_HPP
