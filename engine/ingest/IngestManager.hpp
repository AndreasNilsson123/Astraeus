#ifndef ASTRAEUS_INGEST_MANAGER_HPP
#define ASTRAEUS_INGEST_MANAGER_HPP

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <mutex>
#include <atomic>

#include "SchemaRegistry.hpp"
#include "SnapshotStore.hpp"
#include "TimeSync.hpp"
#include "WorldSync.hpp"
#include "FixedBinaryDecoder.hpp"

namespace astraeus {

class World;

/**
 * Ingest job status for tracking ongoing ingestion operations.
 */
struct IngestJobStatus {
    uint64_t job_id;
    uint32_t format;
    uint32_t total_bytes;
    uint32_t processed_bytes;
    bool is_complete;
    bool has_error;
    char last_error[256];
    
    IngestJobStatus() 
        : job_id(0)
        , format(0)
        , total_bytes(0)
        , processed_bytes(0)
        , is_complete(false)
        , has_error(false)
    {
        last_error[0] = '\0';
    }
};

/**
 * IngestManager handles external data ingestion from physics/simulation systems.
 * Manages the complete ingest pipeline:
 * - SchemaRegistry: Manages format decoders
 * - SnapshotStore: Double-buffered snapshot storage
 * - TimeSync: Simulation time tracking
 * - WorldSync: Applies snapshots to World
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
     * @param format Format identifier (0=FixedBinary, 1=JSON, etc.)
     * @return Job ID (0 if failed)
     */
    uint64_t ingest(const void* data, uint32_t size, uint32_t format);

    /**
     * Get status of an ingest job.
     * @param job_id Job identifier
     * @param out_status Output status struct
     * @return true if job exists, false otherwise
     */
    bool get_job_status(uint64_t job_id, IngestJobStatus* out_status);

    /**
     * Get the last completed snapshot and apply to World.
     * This should be called once per frame to update entities.
     */
    void update();

    /**
     * Get statistics.
     */
    uint64_t get_total_jobs() const { return next_job_id_.load() - 1; }
    uint64_t get_snapshot_count() const { return snapshot_store_ ? snapshot_store_->get_swap_count() : 0; }
    double get_sim_time() const { return time_sync_ ? time_sync_->get_sim_time() : 0.0; }

private:
    World* world_;
    bool is_initialized_;
    
    // Ingest pipeline components
    std::unique_ptr<SchemaRegistry> schema_registry_;
    std::unique_ptr<SnapshotStore> snapshot_store_;
    std::unique_ptr<TimeSync> time_sync_;
    std::unique_ptr<WorldSync> world_sync_;
    
    // Job tracking
    std::atomic<uint64_t> next_job_id_;
    IngestJobStatus last_job_;
    mutable std::mutex job_mutex_;
    
    // Configuration
    static constexpr uint32_t MAX_ENTITIES = 100000;
};

// Inline implementations

inline IngestManager::IngestManager(World* world)
    : world_(world)
    , is_initialized_(false)
    , next_job_id_(1)
{
}

inline IngestManager::~IngestManager() {
    shutdown();
}

inline bool IngestManager::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[IngestManager] Initializing with capacity for " << MAX_ENTITIES << " entities" << std::endl;
    
    if (!world_) {
        std::cerr << "[IngestManager] Cannot initialize with null World" << std::endl;
        return false;
    }
    
    try {
        // Create schema registry
        schema_registry_ = std::make_unique<SchemaRegistry>();
        if (!schema_registry_->initialize()) {
            std::cerr << "[IngestManager] Failed to initialize SchemaRegistry" << std::endl;
            return false;
        }
        
        // Register default decoders
        auto fixed_binary_decoder = std::make_shared<FixedBinaryDecoder>();
        schema_registry_->register_schema(
            0,  // Schema ID 0 = FixedBinary
            "FixedBinary",
            "1.0",
            sizeof(FixedBinaryDecoder::EntityData),
            fixed_binary_decoder
        );
        
        // Create snapshot store
        snapshot_store_ = std::make_unique<SnapshotStore>();
        if (!snapshot_store_->initialize(MAX_ENTITIES)) {
            std::cerr << "[IngestManager] Failed to initialize SnapshotStore" << std::endl;
            return false;
        }
        
        // Create time sync
        time_sync_ = std::make_unique<TimeSync>();
        if (!time_sync_->initialize()) {
            std::cerr << "[IngestManager] Failed to initialize TimeSync" << std::endl;
            return false;
        }
        time_sync_->set_target_rate(10.0); // Default 10 Hz
        
        // Create world sync
        world_sync_ = std::make_unique<WorldSync>(world_);
        if (!world_sync_->initialize()) {
            std::cerr << "[IngestManager] Failed to initialize WorldSync" << std::endl;
            return false;
        }
        
        is_initialized_ = true;
        std::cout << "[IngestManager] Initialized successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[IngestManager] Exception during initialization: " << e.what() << std::endl;
        return false;
    }
}

inline void IngestManager::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[IngestManager] Shutting down" << std::endl;
    
    if (world_sync_) {
        world_sync_->shutdown();
        world_sync_.reset();
    }
    
    if (time_sync_) {
        time_sync_->shutdown();
        time_sync_.reset();
    }
    
    if (snapshot_store_) {
        snapshot_store_->shutdown();
        snapshot_store_.reset();
    }
    
    if (schema_registry_) {
        schema_registry_->shutdown();
        schema_registry_.reset();
    }
    
    is_initialized_ = false;
}

inline uint64_t IngestManager::ingest(const void* data, uint32_t size, uint32_t format) {
    if (!is_initialized_ || !data || size == 0) {
        return 0;
    }
    
    // Allocate job ID
    uint64_t job_id = next_job_id_++;
    
    // Update job status
    {
        std::lock_guard<std::mutex> lock(job_mutex_);
        last_job_.job_id = job_id;
        last_job_.format = format;
        last_job_.total_bytes = size;
        last_job_.processed_bytes = 0;
        last_job_.is_complete = false;
        last_job_.has_error = false;
        last_job_.last_error[0] = '\0';
    }
    
    // Get decoder for format
    auto decoder = schema_registry_->get_decoder(format);
    if (!decoder) {
        std::lock_guard<std::mutex> lock(job_mutex_);
        last_job_.has_error = true;
        std::snprintf(last_job_.last_error, sizeof(last_job_.last_error), 
                     "No decoder registered for format %u", format);
        last_job_.is_complete = true;
        std::cerr << "[IngestManager] " << last_job_.last_error << std::endl;
        return job_id;
    }
    
    // Decode data into snapshot store
    bool success = decoder->decode(data, size, snapshot_store_.get(), time_sync_.get());
    
    // Update job status
    {
        std::lock_guard<std::mutex> lock(job_mutex_);
        last_job_.processed_bytes = size;
        last_job_.is_complete = true;
        
        if (!success) {
            last_job_.has_error = true;
            std::snprintf(last_job_.last_error, sizeof(last_job_.last_error), 
                         "Decoder failed to process data");
            std::cerr << "[IngestManager] Job " << job_id << " failed: " << last_job_.last_error << std::endl;
        } else {
            std::cout << "[IngestManager] Job " << job_id << " completed: " 
                     << size << " bytes ingested (format=" << format << ")" << std::endl;
        }
    }
    
    return job_id;
}

inline bool IngestManager::get_job_status(uint64_t job_id, IngestJobStatus* out_status) {
    if (!out_status) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(job_mutex_);
    
    // Only report on the last job for now (simple single-job tracking)
    if (job_id != last_job_.job_id) {
        return false;
    }
    
    *out_status = last_job_;
    return true;
}

inline void IngestManager::update() {
    if (!is_initialized_ || !world_sync_ || !snapshot_store_) {
        return;
    }
    
    // Get latest snapshot and apply to world
    SnapshotView snapshot = snapshot_store_->get_latest_snapshot();
    if (snapshot.is_valid()) {
        world_sync_->apply_snapshot(snapshot);
    }
}

} // namespace astraeus

#endif // ASTRAEUS_INGEST_MANAGER_HPP
