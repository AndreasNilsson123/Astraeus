#ifndef ASTRAEUS_COMMAND_BUFFER_HPP
#define ASTRAEUS_COMMAND_BUFFER_HPP

#include <cstdint>
#include <vector>
#include <mutex>
#include <memory>
#include <cstring>

namespace astraeus {

// Forward declarations
class EngineContext;
class World;

/**
 * Command types for deterministic execution.
 * Commands are submitted from any thread and executed on engine tick.
 */
enum class CommandType : uint32_t {
    CreateEntity = 0,
    DestroyEntity = 1,
    SetTransform = 2,
    AssignMesh = 3,
    AssignMaterial = 4,
    SetTrailParams = 5,
    SetEntityColor = 6,
    SetEntityVisible = 7,
    ApplySnapshot = 8
};

/**
 * Base command structure.
 * All commands inherit from this and provide data payload.
 */
struct Command {
    CommandType type;
    uint32_t entity_id;  // Target entity (if applicable)
    
    Command() : type(CommandType::CreateEntity), entity_id(0) {}
    explicit Command(CommandType t, uint32_t eid = 0) : type(t), entity_id(eid) {}
    virtual ~Command() = default;
};

/**
 * Create entity command.
 * Returns new entity ID via callback on execution.
 */
struct CreateEntityCommand : public Command {
    uint32_t* out_entity_id;  // Output: pointer to receive new entity ID
    
    CreateEntityCommand() : Command(CommandType::CreateEntity), out_entity_id(nullptr) {}
};

/**
 * Destroy entity command.
 */
struct DestroyEntityCommand : public Command {
    DestroyEntityCommand() : Command(CommandType::DestroyEntity) {}
    explicit DestroyEntityCommand(uint32_t eid) : Command(CommandType::DestroyEntity, eid) {}
};

/**
 * Set transform command.
 */
struct SetTransformCommand : public Command {
    float pos_x, pos_y, pos_z;
    float rot_x, rot_y, rot_z;
    float scale_x, scale_y, scale_z;
    
    SetTransformCommand() : Command(CommandType::SetTransform) {}
    SetTransformCommand(uint32_t eid,
                        float px, float py, float pz,
                        float rx, float ry, float rz,
                        float sx, float sy, float sz)
        : Command(CommandType::SetTransform, eid)
        , pos_x(px), pos_y(py), pos_z(pz)
        , rot_x(rx), rot_y(ry), rot_z(rz)
        , scale_x(sx), scale_y(sy), scale_z(sz)
    {}
};

/**
 * Assign mesh command.
 */
struct AssignMeshCommand : public Command {
    uint32_t mesh_id;
    
    AssignMeshCommand() : Command(CommandType::AssignMesh), mesh_id(0) {}
    AssignMeshCommand(uint32_t eid, uint32_t mid)
        : Command(CommandType::AssignMesh, eid), mesh_id(mid) {}
};

/**
 * Assign material command.
 */
struct AssignMaterialCommand : public Command {
    uint32_t material_id;
    
    AssignMaterialCommand() : Command(CommandType::AssignMaterial), material_id(0) {}
    AssignMaterialCommand(uint32_t eid, uint32_t mid)
        : Command(CommandType::AssignMaterial, eid), material_id(mid) {}
};

/**
 * Set trail parameters command.
 */
struct SetTrailParamsCommand : public Command {
    uint32_t max_points;
    
    SetTrailParamsCommand() : Command(CommandType::SetTrailParams), max_points(0) {}
    SetTrailParamsCommand(uint32_t eid, uint32_t max_pts)
        : Command(CommandType::SetTrailParams, eid), max_points(max_pts) {}
};

/**
 * Set entity color command.
 */
struct SetEntityColorCommand : public Command {
    float r, g, b, a;
    
    SetEntityColorCommand() : Command(CommandType::SetEntityColor) {}
    SetEntityColorCommand(uint32_t eid, float red, float green, float blue, float alpha)
        : Command(CommandType::SetEntityColor, eid), r(red), g(green), b(blue), a(alpha) {}
};

/**
 * Set entity visible command.
 */
struct SetEntityVisibleCommand : public Command {
    bool visible;
    
    SetEntityVisibleCommand() : Command(CommandType::SetEntityVisible), visible(true) {}
    SetEntityVisibleCommand(uint32_t eid, bool vis)
        : Command(CommandType::SetEntityVisible, eid), visible(vis) {}
};

/**
 * Apply snapshot command (for WorldSync).
 */
struct ApplySnapshotCommand : public Command {
    float pos_x, pos_y, pos_z;
    
    ApplySnapshotCommand() : Command(CommandType::ApplySnapshot) {}
    ApplySnapshotCommand(uint32_t eid, float px, float py, float pz)
        : Command(CommandType::ApplySnapshot, eid), pos_x(px), pos_y(py), pos_z(pz) {}
};

/**
 * CommandBuffer manages a queue of commands for deterministic execution.
 * Thread-safe: commands can be submitted from any thread.
 * Commands are executed during engine tick in submission order.
 */
class CommandBuffer {
public:
    CommandBuffer();
    ~CommandBuffer();

    /**
     * Submit a command for execution.
     * Thread-safe. Commands are queued and executed on next tick.
     */
    void submit(std::unique_ptr<Command> command);

    /**
     * Execute all pending commands.
     * Called by EngineContext during tick().
     * Not thread-safe - must be called from engine thread only.
     */
    void execute(EngineContext* context);  // Implemented in EngineContext.hpp to avoid circular dependency

    /**
     * Clear all pending commands.
     * Useful for emergency reset.
     */
    void clear();

    /**
     * Get number of pending commands.
     */
    size_t pending_count() const;

private:
    // Double-buffering: submit queue and execute queue
    std::vector<std::unique_ptr<Command>> submit_queue_;
    std::vector<std::unique_ptr<Command>> execute_queue_;
    mutable std::mutex mutex_;
};

// =============================================================================
// INLINE IMPLEMENTATIONS
// =============================================================================

inline CommandBuffer::CommandBuffer() {
    submit_queue_.reserve(256);
    execute_queue_.reserve(256);
}

inline CommandBuffer::~CommandBuffer() {
    clear();
}

inline void CommandBuffer::submit(std::unique_ptr<Command> command) {
    if (!command) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    submit_queue_.push_back(std::move(command));
}

inline void CommandBuffer::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    submit_queue_.clear();
    execute_queue_.clear();
}

inline size_t CommandBuffer::pending_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return submit_queue_.size() + execute_queue_.size();
}



} // namespace astraeus

#endif // ASTRAEUS_COMMAND_BUFFER_HPP
