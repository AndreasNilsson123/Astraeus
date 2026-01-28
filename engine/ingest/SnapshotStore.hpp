#ifndef ASTRAEUS_SNAPSHOT_STORE_HPP
#define ASTRAEUS_SNAPSHOT_STORE_HPP

#include "SnapshotView.h"
#include <vector>
#include <mutex>
#include <atomic>
#include <iostream>
#include <cstring>

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

// Inline implementations

inline SnapshotStore::SnapshotStore()
    : max_entities_(0)
    , is_initialized_(false)
    , is_writing_(false)
    , read_index_(0)
    , write_index_(1)
    , swap_count_(0)
{
}

inline SnapshotStore::~SnapshotStore() {
    shutdown();
}

inline bool SnapshotStore::initialize(uint32_t max_entities) {
    if (is_initialized_) {
        return true;
    }
    
    max_entities_ = max_entities;
    
    // Pre-allocate both buffers to avoid reallocations during runtime
    for (int i = 0; i < 2; i++) {
        buffers_[i].entities.reserve(max_entities);
        buffers_[i].metadata.reserve(max_entities);
    }
    
    is_initialized_ = true;
    std::cout << "[SnapshotStore] Initialized with capacity for " << max_entities << " entities" << std::endl;
    return true;
}

inline void SnapshotStore::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(write_mutex_);
    
    buffers_[0].entities.clear();
    buffers_[0].metadata.clear();
    buffers_[1].entities.clear();
    buffers_[1].metadata.clear();
    
    is_initialized_ = false;
    is_writing_ = false;
    std::cout << "[SnapshotStore] Shutdown" << std::endl;
}

inline bool SnapshotStore::begin_write(double timestamp, uint64_t frame_number) {
    if (!is_initialized_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(write_mutex_);
    
    if (is_writing_) {
        std::cerr << "[SnapshotStore] Warning: begin_write called while already writing" << std::endl;
        return false;
    }
    
    // Clear write buffer
    Buffer& write_buf = buffers_[write_index_];
    write_buf.entities.clear();
    write_buf.metadata.clear();
    write_buf.timestamp = timestamp;
    write_buf.frame_number = frame_number;
    write_buf.entity_count = 0;
    
    is_writing_ = true;
    return true;
}

inline void SnapshotStore::write_entity(const EntitySnapshot& entity, const EntityMetadata& metadata) {
    if (!is_writing_) {
        std::cerr << "[SnapshotStore] Error: write_entity called without begin_write" << std::endl;
        return;
    }
    
    std::lock_guard<std::mutex> lock(write_mutex_);
    
    Buffer& write_buf = buffers_[write_index_];
    
    // Check capacity
    if (write_buf.entities.size() >= max_entities_) {
        std::cerr << "[SnapshotStore] Warning: max entities reached, dropping entity" << std::endl;
        return;
    }
    
    write_buf.entities.push_back(entity);
    write_buf.metadata.push_back(metadata);
    write_buf.entity_count++;
}

inline void SnapshotStore::end_write() {
    if (!is_writing_) {
        std::cerr << "[SnapshotStore] Error: end_write called without begin_write" << std::endl;
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(write_mutex_);
        is_writing_ = false;
        
        // Atomic swap: write buffer becomes read buffer
        int old_read = read_index_.load();
        read_index_.store(write_index_);
        write_index_ = old_read;
        
        swap_count_++;
    }
}

inline SnapshotView SnapshotStore::get_latest_snapshot() const {
    if (!is_initialized_) {
        return SnapshotView();
    }
    
    // Load read index atomically (no lock needed)
    int idx = read_index_.load();
    const Buffer& read_buf = buffers_[idx];
    
    if (read_buf.entity_count == 0) {
        return SnapshotView();
    }
    
    return SnapshotView(
        read_buf.timestamp,
        read_buf.frame_number,
        read_buf.entity_count,
        read_buf.entities.data(),
        read_buf.metadata.data()
    );
}

} // namespace astraeus

#endif // ASTRAEUS_SNAPSHOT_STORE_HPP
