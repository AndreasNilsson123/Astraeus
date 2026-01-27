#include "FixedBinaryDecoder.hpp"
#include "SnapshotStore.hpp"
#include "TimeSync.hpp"
#include <cstring>
#include <iostream>

namespace astraeus {

FixedBinaryDecoder::FixedBinaryDecoder() {
}

const char* FixedBinaryDecoder::get_name() const {
    return "FixedBinaryDecoder";
}

bool FixedBinaryDecoder::validate(const void* data, uint32_t size) const {
    if (!data || size < sizeof(Header)) {
        return false;
    }
    
    const Header* header = static_cast<const Header*>(data);
    
    // Check magic number
    if (header->magic != MAGIC) {
        return false;
    }
    
    // Check version
    if (header->version != VERSION) {
        std::cerr << "[FixedBinaryDecoder] Unsupported version: " << header->version << std::endl;
        return false;
    }
    
    // Check size
    uint32_t expected_size = sizeof(Header) +
                            (header->entity_count * sizeof(EntityData)) +
                            (header->metadata_count * sizeof(MetadataData));
    
    if (size < expected_size) {
        std::cerr << "[FixedBinaryDecoder] Invalid size: got " << size 
                  << ", expected " << expected_size << std::endl;
        return false;
    }
    
    return true;
}

bool FixedBinaryDecoder::decode(const void* data, uint32_t size,
                                SnapshotStore* store, TimeSync* time_sync) {
    if (!validate(data, size)) {
        return false;
    }
    
    if (!store || !time_sync) {
        std::cerr << "[FixedBinaryDecoder] Null store or time_sync" << std::endl;
        return false;
    }
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    const Header* header = reinterpret_cast<const Header*>(bytes);
    
    // Update time sync
    time_sync->update_sim_time(header->timestamp);
    time_sync->advance_frame();
    
    // Begin writing snapshot
    if (!store->begin_write(header->timestamp, header->frame_number)) {
        return false;
    }
    
    // Decode entities
    const EntityData* entities = reinterpret_cast<const EntityData*>(
        bytes + sizeof(Header)
    );
    
    // Decode metadata
    const MetadataData* metadata = reinterpret_cast<const MetadataData*>(
        bytes + sizeof(Header) + (header->entity_count * sizeof(EntityData))
    );
    
    for (uint32_t i = 0; i < header->entity_count; i++) {
        const EntityData& src = entities[i];
        
        EntitySnapshot entity_snap;
        entity_snap.entity_id = src.entity_id;
        entity_snap.pos_x = src.pos_x;
        entity_snap.pos_y = src.pos_y;
        entity_snap.pos_z = src.pos_z;
        entity_snap.rot_x = src.rot_x;
        entity_snap.rot_y = src.rot_y;
        entity_snap.rot_z = src.rot_z;
        entity_snap.scale_x = src.scale_x;
        entity_snap.scale_y = src.scale_y;
        entity_snap.scale_z = src.scale_z;
        entity_snap.color_r = src.color_r;
        entity_snap.color_g = src.color_g;
        entity_snap.color_b = src.color_b;
        entity_snap.color_a = src.color_a;
        entity_snap.metadata_index = src.metadata_index;
        entity_snap.active = (src.active != 0);
        
        // Get metadata (if valid index)
        EntityMetadata entity_meta;
        if (src.metadata_index < header->metadata_count) {
            const MetadataData& meta_src = metadata[src.metadata_index];
            std::strncpy(entity_meta.name, meta_src.name, sizeof(entity_meta.name) - 1);
            std::strncpy(entity_meta.team, meta_src.team, sizeof(entity_meta.team) - 1);
            std::strncpy(entity_meta.type, meta_src.type, sizeof(entity_meta.type) - 1);
            entity_meta.name[sizeof(entity_meta.name) - 1] = '\0';
            entity_meta.team[sizeof(entity_meta.team) - 1] = '\0';
            entity_meta.type[sizeof(entity_meta.type) - 1] = '\0';
        }
        
        store->write_entity(entity_snap, entity_meta);
    }
    
    // Finish writing and swap buffers
    store->end_write();
    
    return true;
}

} // namespace astraeus
