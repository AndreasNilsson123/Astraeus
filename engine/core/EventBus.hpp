#ifndef ASTRAEUS_EVENT_BUS_HPP
#define ASTRAEUS_EVENT_BUS_HPP

#include <cstdint>
#include <vector>
#include <mutex>
#include <cstring>
#include <algorithm>
#include <chrono>

namespace astraeus {

/**
 * Event types for native-side event system.
 * Events are posted by engine and polled by Java (no callbacks).
 */
enum class EventType : uint32_t {
    SelectionChanged = 0,
    AssetLoaded = 1,
    IngestStarted = 2,
    IngestProgress = 3,
    IngestCompleted = 4,
    EntityCreated = 5,
    EntityDestroyed = 6
};

/**
 * Base event structure.
 * All events inherit from this with specific data payload.
 */
struct Event {
    EventType type;
    uint64_t timestamp_ns;  // Nanosecond timestamp when event was posted
    
    Event() : type(EventType::SelectionChanged), timestamp_ns(0) {}
    explicit Event(EventType t) : type(t), timestamp_ns(0) {}
    virtual ~Event() = default;
};

/**
 * Selection changed event.
 * Posted when entity selection changes in the engine.
 */
struct SelectionChangedEvent : public Event {
    uint32_t entity_id;     // Selected entity ID (0 = none)
    float world_x, world_y, world_z;  // World position of selection
    
    SelectionChangedEvent() : Event(EventType::SelectionChanged), entity_id(0),
        world_x(0), world_y(0), world_z(0) {}
    SelectionChangedEvent(uint32_t eid, float wx, float wy, float wz)
        : Event(EventType::SelectionChanged), entity_id(eid),
          world_x(wx), world_y(wy), world_z(wz) {}
};

/**
 * Asset loaded event.
 * Posted when an asset (mesh, texture, material) is loaded.
 */
struct AssetLoadedEvent : public Event {
    uint32_t asset_id;      // Asset ID/handle
    uint32_t asset_type;    // Asset type (0=mesh, 1=texture, 2=material)
    char asset_name[256];   // Asset name/path
    
    AssetLoadedEvent() : Event(EventType::AssetLoaded), asset_id(0), asset_type(0) {
        asset_name[0] = '\0';
    }
    AssetLoadedEvent(uint32_t aid, uint32_t atype, const char* name)
        : Event(EventType::AssetLoaded), asset_id(aid), asset_type(atype) {
        if (name) {
            snprintf(asset_name, sizeof(asset_name), "%s", name);
            asset_name[sizeof(asset_name) - 1] = '\0';
        } else {
            asset_name[0] = '\0';
        }
    }
};

/**
 * Ingest started event.
 * Posted when data ingestion begins.
 */
struct IngestStartedEvent : public Event {
    uint32_t format;        // Data format ID
    uint32_t total_bytes;   // Total bytes to ingest
    
    IngestStartedEvent() : Event(EventType::IngestStarted), format(0), total_bytes(0) {}
    IngestStartedEvent(uint32_t fmt, uint32_t bytes)
        : Event(EventType::IngestStarted), format(fmt), total_bytes(bytes) {}
};

/**
 * Ingest progress event.
 * Posted periodically during data ingestion.
 */
struct IngestProgressEvent : public Event {
    uint32_t bytes_processed;   // Bytes processed so far
    uint32_t total_bytes;       // Total bytes to process
    float progress;             // Progress [0.0, 1.0]
    
    IngestProgressEvent() : Event(EventType::IngestProgress),
        bytes_processed(0), total_bytes(0), progress(0.0f) {}
    IngestProgressEvent(uint32_t processed, uint32_t total)
        : Event(EventType::IngestProgress),
          bytes_processed(processed), total_bytes(total),
          progress(total > 0 ? static_cast<float>(processed) / total : 0.0f) {}
};

/**
 * Ingest completed event.
 * Posted when data ingestion completes (success or failure).
 */
struct IngestCompletedEvent : public Event {
    bool success;           // Whether ingestion succeeded
    uint32_t entities_created;  // Number of entities created
    char error_message[256];    // Error message (if failed)
    
    IngestCompletedEvent() : Event(EventType::IngestCompleted),
        success(false), entities_created(0) {
        error_message[0] = '\0';
    }
    IngestCompletedEvent(bool ok, uint32_t count, const char* error = nullptr)
        : Event(EventType::IngestCompleted), success(ok), entities_created(count) {
        if (error) {
            snprintf(error_message, sizeof(error_message), "%s", error);
            error_message[sizeof(error_message) - 1] = '\0';
        } else {
            error_message[0] = '\0';
        }
    }
};

/**
 * Entity created event.
 * Posted when a new entity is created.
 */
struct EntityCreatedEvent : public Event {
    uint32_t entity_id;
    
    EntityCreatedEvent() : Event(EventType::EntityCreated), entity_id(0) {}
    explicit EntityCreatedEvent(uint32_t eid)
        : Event(EventType::EntityCreated), entity_id(eid) {}
};

/**
 * Entity destroyed event.
 * Posted when an entity is destroyed.
 */
struct EntityDestroyedEvent : public Event {
    uint32_t entity_id;
    
    EntityDestroyedEvent() : Event(EventType::EntityDestroyed), entity_id(0) {}
    explicit EntityDestroyedEvent(uint32_t eid)
        : Event(EventType::EntityDestroyed), entity_id(eid) {}
};

/**
 * EventBus manages event queue for native-side event system.
 * Thread-safe: events can be posted from any thread.
 * Events are polled by Java (no callbacks, FFM-safe).
 */
class EventBus {
public:
    EventBus();
    ~EventBus();

    /**
     * Post an event to the bus.
     * Thread-safe. Events are queued for polling.
     */
    void post(Event* event);

    /**
     * Poll next event from the bus.
     * Returns nullptr if no events are pending.
     * Caller is responsible for deleting the returned event.
     * Thread-safe but should be called from single consumer thread.
     */
    Event* poll();

    /**
     * Peek at next event without removing it.
     * Returns nullptr if no events are pending.
     * Thread-safe.
     */
    const Event* peek() const;

    /**
     * Get number of pending events.
     */
    size_t pending_count() const;

    /**
     * Clear all pending events.
     * Useful for emergency reset.
     */
    void clear();

    /**
     * Set maximum event queue size.
     * Older events are dropped if queue exceeds this size.
     */
    void set_max_queue_size(size_t max_size);

private:
    std::vector<Event*> event_queue_;
    mutable std::mutex mutex_;
    size_t max_queue_size_;
    
    // Get current timestamp in nanoseconds
    uint64_t get_timestamp_ns() const;
};

// =============================================================================
// INLINE IMPLEMENTATIONS
// =============================================================================

inline EventBus::EventBus()
    : max_queue_size_(1024) {
    event_queue_.reserve(256);
}

inline EventBus::~EventBus() {
    clear();
}

inline void EventBus::post(Event* event) {
    if (!event) {
        return;
    }
    
    // Set timestamp if not already set
    if (event->timestamp_ns == 0) {
        event->timestamp_ns = get_timestamp_ns();
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Drop oldest events if queue is full
    while (event_queue_.size() >= max_queue_size_) {
        Event* old_event = event_queue_.front();
        event_queue_.erase(event_queue_.begin());
        delete old_event;
    }
    
    event_queue_.push_back(event);
}

inline Event* EventBus::poll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (event_queue_.empty()) {
        return nullptr;
    }
    
    Event* event = event_queue_.front();
    event_queue_.erase(event_queue_.begin());
    return event;
}

inline const Event* EventBus::peek() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (event_queue_.empty()) {
        return nullptr;
    }
    
    return event_queue_.front();
}

inline size_t EventBus::pending_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return event_queue_.size();
}

inline void EventBus::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (Event* event : event_queue_) {
        delete event;
    }
    event_queue_.clear();
}

inline void EventBus::set_max_queue_size(size_t max_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_queue_size_ = max_size;
}

inline uint64_t EventBus::get_timestamp_ns() const {
    // Use high-resolution clock for timestamps
    // This is a simple implementation; production code might use platform-specific timers
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

} // namespace astraeus

#endif // ASTRAEUS_EVENT_BUS_HPP
