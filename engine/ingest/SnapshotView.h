#ifndef ASTRAEUS_SNAPSHOT_VIEW_H
#define ASTRAEUS_SNAPSHOT_VIEW_H

#include <cstdint>
#include <string>

namespace astraeus {

/**
 * Entity data in a snapshot (POD for efficient copying).
 */
struct EntitySnapshot {
    uint32_t entity_id;
    float pos_x, pos_y, pos_z;
    float rot_x, rot_y, rot_z;
    float scale_x, scale_y, scale_z;
    float color_r, color_g, color_b, color_a;
    uint32_t metadata_index; // Index into metadata array
    bool active;
    
    EntitySnapshot()
        : entity_id(0)
        , pos_x(0), pos_y(0), pos_z(0)
        , rot_x(0), rot_y(0), rot_z(0)
        , scale_x(1), scale_y(1), scale_z(1)
        , color_r(1), color_g(1), color_b(1), color_a(1)
        , metadata_index(0)
        , active(false)
    {}
};

/**
 * Entity metadata (name, team, type).
 */
struct EntityMetadata {
    char name[64];
    char team[32];
    char type[32];
    
    EntityMetadata() {
        name[0] = '\0';
        team[0] = '\0';
        type[0] = '\0';
    }
};

/**
 * Read-only view of a complete snapshot.
 * Used by WorldSync and render thread to consume snapshot data.
 */
class SnapshotView {
public:
    SnapshotView()
        : timestamp_(0.0)
        , frame_number_(0)
        , entity_count_(0)
        , entities_(nullptr)
        , metadata_(nullptr)
    {}
    
    SnapshotView(double timestamp, uint64_t frame_number,
                 uint32_t entity_count,
                 const EntitySnapshot* entities,
                 const EntityMetadata* metadata)
        : timestamp_(timestamp)
        , frame_number_(frame_number)
        , entity_count_(entity_count)
        , entities_(entities)
        , metadata_(metadata)
    {}
    
    // Accessors
    double get_timestamp() const { return timestamp_; }
    uint64_t get_frame_number() const { return frame_number_; }
    uint32_t get_entity_count() const { return entity_count_; }
    
    const EntitySnapshot* get_entities() const { return entities_; }
    const EntitySnapshot* get_entity(uint32_t index) const {
        if (index >= entity_count_) return nullptr;
        return &entities_[index];
    }
    
    const EntityMetadata* get_metadata() const { return metadata_; }
    const EntityMetadata* get_entity_metadata(uint32_t index) const {
        if (index >= entity_count_) return nullptr;
        return &metadata_[index];
    }
    
    bool is_valid() const { return entities_ != nullptr; }
    
private:
    double timestamp_;
    uint64_t frame_number_;
    uint32_t entity_count_;
    const EntitySnapshot* entities_;
    const EntityMetadata* metadata_;
};

} // namespace astraeus

#endif // ASTRAEUS_SNAPSHOT_VIEW_H
