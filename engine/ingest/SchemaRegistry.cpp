#include "SchemaRegistry.hpp"
#include "Decoder.h"
#include <iostream>

namespace astraeus {

SchemaRegistry::SchemaRegistry()
    : is_initialized_(false)
{
}

SchemaRegistry::~SchemaRegistry() {
    shutdown();
}

bool SchemaRegistry::initialize() {
    if (is_initialized_) {
        return true;
    }
    
    is_initialized_ = true;
    std::cout << "[SchemaRegistry] Initialized" << std::endl;
    return true;
}

void SchemaRegistry::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    schemas_.clear();
    decoders_.clear();
    
    is_initialized_ = false;
    std::cout << "[SchemaRegistry] Shutdown" << std::endl;
}

bool SchemaRegistry::register_schema(uint32_t schema_id,
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

std::shared_ptr<Decoder> SchemaRegistry::get_decoder(uint32_t schema_id) const {
    auto it = decoders_.find(schema_id);
    if (it != decoders_.end()) {
        return it->second;
    }
    return nullptr;
}

const SchemaInfo* SchemaRegistry::get_schema_info(uint32_t schema_id) const {
    auto it = schemas_.find(schema_id);
    if (it != schemas_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool SchemaRegistry::has_schema(uint32_t schema_id) const {
    return schemas_.find(schema_id) != schemas_.end();
}

} // namespace astraeus
