#ifndef ASTRAEUS_INGEST_MANAGER_HPP
#define ASTRAEUS_INGEST_MANAGER_HPP

#include <cstdint>

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

} // namespace astraeus

#endif // ASTRAEUS_INGEST_MANAGER_HPP
