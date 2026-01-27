#ifndef ASTRAEUS_SCHEMA_REGISTRY_HPP
#define ASTRAEUS_SCHEMA_REGISTRY_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

namespace astraeus {

class Decoder;

/**
 * Schema information for a data format.
 */
struct SchemaInfo {
    uint32_t schema_id;
    std::string name;
    std::string version;
    uint32_t entity_size_bytes;
    
    SchemaInfo()
        : schema_id(0), entity_size_bytes(0)
    {}
};

/**
 * SchemaRegistry manages data format schemas and decoders.
 * Maps schema IDs to decoder implementations.
 */
class SchemaRegistry {
public:
    SchemaRegistry();
    ~SchemaRegistry();
    
    /**
     * Initialize registry.
     */
    bool initialize();
    
    /**
     * Shutdown.
     */
    void shutdown();
    
    /**
     * Register a schema with its decoder.
     */
    bool register_schema(uint32_t schema_id, 
                        const std::string& name,
                        const std::string& version,
                        uint32_t entity_size_bytes,
                        std::shared_ptr<Decoder> decoder);
    
    /**
     * Get decoder for schema ID.
     */
    std::shared_ptr<Decoder> get_decoder(uint32_t schema_id) const;
    
    /**
     * Get schema info.
     */
    const SchemaInfo* get_schema_info(uint32_t schema_id) const;
    
    /**
     * Check if schema is registered.
     */
    bool has_schema(uint32_t schema_id) const;
    
private:
    bool is_initialized_;
    std::unordered_map<uint32_t, SchemaInfo> schemas_;
    std::unordered_map<uint32_t, std::shared_ptr<Decoder>> decoders_;
};

} // namespace astraeus

#endif // ASTRAEUS_SCHEMA_REGISTRY_HPP
