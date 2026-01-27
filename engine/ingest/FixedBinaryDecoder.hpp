#ifndef ASTRAEUS_FIXED_BINARY_DECODER_HPP
#define ASTRAEUS_FIXED_BINARY_DECODER_HPP

#include "Decoder.h"
#include <cstdint>

namespace astraeus {

/**
 * Fixed-layout binary format for entity snapshots.
 * Simple, efficient format for high-frequency updates.
 * 
 * Format:
 * - Header: timestamp (double), frame_number (uint64), entity_count (uint32)
 * - Entities: array of EntityData structs
 */
class FixedBinaryDecoder : public Decoder {
public:
    /**
     * Fixed binary entity data layout (POD).
     */
    #pragma pack(push, 1)
    struct EntityData {
        uint32_t entity_id;
        float pos_x, pos_y, pos_z;
        float rot_x, rot_y, rot_z;
        float scale_x, scale_y, scale_z;
        float color_r, color_g, color_b, color_a;
        uint32_t metadata_index;
        uint8_t active; // boolean as byte
    };
    
    struct MetadataData {
        char name[64];
        char team[32];
        char type[32];
    };
    
    struct Header {
        uint32_t magic;        // 0x41535430 ("AST0")
        uint32_t version;      // Format version
        double timestamp;
        uint64_t frame_number;
        uint32_t entity_count;
        uint32_t metadata_count;
    };
    #pragma pack(pop)
    
    FixedBinaryDecoder();
    virtual ~FixedBinaryDecoder() = default;
    
    // Decoder interface
    bool decode(const void* data, uint32_t size,
               SnapshotStore* store, TimeSync* time_sync) override;
    
    const char* get_name() const override;
    bool validate(const void* data, uint32_t size) const override;
    
    static constexpr uint32_t MAGIC = 0x41535430; // "AST0"
    static constexpr uint32_t VERSION = 1;
};

} // namespace astraeus

#endif // ASTRAEUS_FIXED_BINARY_DECODER_HPP
