#ifndef ASTRAEUS_SNAPSHOT_STORE_HPP
#define ASTRAEUS_SNAPSHOT_STORE_HPP

#include "SnapshotView.h"
#include <vector>
#include <mutex>
#include <atomic>

namespace astraeus {

/**
 * SnapshotStore manages double-buffered snapshots for thread-safe data ingestion.
 * 
 * Design:
 * - Write buffer: Decoders write new snapshot data here
 * - Read buffer: Renderer/WorldSync reads from here
 * - Atomic swap ensures no partial reads
 * 
 * Thread Safety:
 * - Write operations are mutex-protected
 * - Read operations use atomic pointer to get stable snapshot
 * - Swap is atomic and lock-free for readers
 */
class SnapshotStore {
public:
    SnapshotStore();
    ~SnapshotStore();
    
    /**
     * Initialize the store with maximum entity capacity.
     */
    bool initialize(uint32_t max_entities);
    
    /**
     * Shutdown and release resources.
     */
    void shutdown();
    
    /**
     * Begin writing a new snapshot (locks write buffer).
     * Returns false if already writing.
     */
    bool begin_write(double timestamp, uint64_t frame_number);
    
    /**
     * Write entity data to current snapshot.
     */
    void write_entity(const EntitySnapshot& entity, const EntityMetadata& metadata);
    
    /**
     * Finish writing and swap buffers atomically.
     * Makes new snapshot available for reading.
     */
    void end_write();
    
    /**
     * Get the latest completed snapshot for reading.
     * Returns a view that remains stable until next swap.
     */
    SnapshotView get_latest_snapshot() const;
    
    /**
     * Get statistics.
     */
    uint32_t get_max_entities() const { return max_entities_; }
    uint64_t get_swap_count() const { return swap_count_.load(); }
    
private:
    struct Buffer {
        double timestamp;
        uint64_t frame_number;
        uint32_t entity_count;
        std::vector<EntitySnapshot> entities;
        std::vector<EntityMetadata> metadata;
        
        Buffer() : timestamp(0.0), frame_number(0), entity_count(0) {}
    };
    
    uint32_t max_entities_;
    bool is_initialized_;
    bool is_writing_;
    
    // Double buffering
    Buffer buffers_[2];
    std::atomic<int> read_index_; // 0 or 1
    int write_index_; // Protected by write_mutex_
    
    mutable std::mutex write_mutex_;
    std::atomic<uint64_t> swap_count_;
};

} // namespace astraeus

#endif // ASTRAEUS_SNAPSHOT_STORE_HPP
