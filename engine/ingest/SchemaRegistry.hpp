#ifndef ASTRAEUS_SCHEMA_REGISTRY_HPP
#define ASTRAEUS_SCHEMA_REGISTRY_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

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

// Inline implementations

inline SchemaRegistry::SchemaRegistry()
    : is_initialized_(false)
{
}

inline SchemaRegistry::~SchemaRegistry() {
    shutdown();
}

inline bool SchemaRegistry::initialize() {
    if (is_initialized_) {
        return true;
    }
    
    is_initialized_ = true;
    std::cout << "[SchemaRegistry] Initialized" << std::endl;
    return true;
}

inline void SchemaRegistry::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    schemas_.clear();
    decoders_.clear();
    
    is_initialized_ = false;
    std::cout << "[SchemaRegistry] Shutdown" << std::endl;
}

inline bool SchemaRegistry::register_schema(uint32_t schema_id,
                                     const std::string& name,
                                     const std::string& version,
                                     uint32_t entity_size_bytes,
                                     std::shared_ptr<Decoder> decoder) {
    if (!is_initialized_) {
        return false;
    }
    
    if (!decoder) {
        std::cerr << "[SchemaRegistry] Cannot register null decoder for schema " << schema_id << std::endl;
        return false;
    }
    
    SchemaInfo info;
    info.schema_id = schema_id;
    info.name = name;
    info.version = version;
    info.entity_size_bytes = entity_size_bytes;
    
    schemas_[schema_id] = info;
    decoders_[schema_id] = decoder;
    
    std::cout << "[SchemaRegistry] Registered schema " << schema_id 
              << " (" << name << " v" << version << ")" << std::endl;
    return true;
}

inline std::shared_ptr<Decoder> SchemaRegistry::get_decoder(uint32_t schema_id) const {
    auto it = decoders_.find(schema_id);
    if (it != decoders_.end()) {
        return it->second;
    }
    return nullptr;
}

inline const SchemaInfo* SchemaRegistry::get_schema_info(uint32_t schema_id) const {
    auto it = schemas_.find(schema_id);
    if (it != schemas_.end()) {
        return &it->second;
    }
    return nullptr;
}

inline bool SchemaRegistry::has_schema(uint32_t schema_id) const {
    return schemas_.find(schema_id) != schemas_.end();
}

} // namespace astraeus

#endif // ASTRAEUS_SCHEMA_REGISTRY_HPP
