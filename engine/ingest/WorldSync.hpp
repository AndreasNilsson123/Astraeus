#ifndef ASTRAEUS_WORLD_SYNC_HPP
#define ASTRAEUS_WORLD_SYNC_HPP

#include "SnapshotView.h"
#include <cstdint>
#include <unordered_set>

namespace astraeus {

class World;

/**
 * WorldSync applies SnapshotView data to the World scene graph.
 * Handles entity creation, updates, and deletion based on snapshot data.
 */
class WorldSync {
public:
    explicit WorldSync(World* world);
    ~WorldSync();
    
    /**
     * Initialize.
     */
    bool initialize();
    
    /**
     * Shutdown.
     */
    void shutdown();
    
    /**
     * Apply a snapshot to the world.
     * Creates/updates/deletes entities as needed.
     */
    void apply_snapshot(const SnapshotView& snapshot);
    
    /**
     * Get statistics.
     */
    uint32_t get_entities_created() const { return entities_created_; }
    uint32_t get_entities_updated() const { return entities_updated_; }
    uint32_t get_entities_deleted() const { return entities_deleted_; }
    
    /**
     * Reset statistics.
     */
    void reset_stats();
    
private:
    World* world_;
    bool is_initialized_;
    
    // Track which entities exist in World (for deletion detection)
    std::unordered_set<uint32_t> active_entities_;
    
    // Statistics
    uint32_t entities_created_;
    uint32_t entities_updated_;
    uint32_t entities_deleted_;
};

} // namespace astraeus

#endif // ASTRAEUS_WORLD_SYNC_HPP
