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
    void execute(EngineContext* context);

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
    
    // Helper to execute individual command
    void execute_command(Command* cmd, EngineContext* context);
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

inline void CommandBuffer::execute(EngineContext* context) {
    if (!context) {
        return;
    }
    
    // Swap queues under lock (fast)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::swap(submit_queue_, execute_queue_);
    }
    
    // Execute commands (lock-free)
    for (auto& cmd : execute_queue_) {
        execute_command(cmd.get(), context);
    }
    
    // Clear executed commands
    execute_queue_.clear();
}

inline void CommandBuffer::execute_command(Command* cmd, EngineContext* context) {
    if (!cmd || !context) {
        return;
    }
    
    switch (cmd->type) {
        case CommandType::CreateEntity: {
            auto* create_cmd = static_cast<CreateEntityCommand*>(cmd);
            uint32_t new_id = context->create_entity();
            if (create_cmd->out_entity_id) {
                *create_cmd->out_entity_id = new_id;
            }
            break;
        }
        
        case CommandType::DestroyEntity: {
            context->destroy_entity(cmd->entity_id);
            break;
        }
        
        case CommandType::SetTransform: {
            auto* trans_cmd = static_cast<SetTransformCommand*>(cmd);
            context->set_entity_transform(cmd->entity_id,
                                         trans_cmd->pos_x, trans_cmd->pos_y, trans_cmd->pos_z,
                                         trans_cmd->rot_x, trans_cmd->rot_y, trans_cmd->rot_z,
                                         trans_cmd->scale_x, trans_cmd->scale_y, trans_cmd->scale_z);
            break;
        }
        
        case CommandType::AssignMesh: {
            // TODO: Implement mesh assignment when asset system supports it
            // auto* mesh_cmd = static_cast<AssignMeshCommand*>(cmd);
            // context->set_entity_mesh(cmd->entity_id, mesh_cmd->mesh_id);
            break;
        }
        
        case CommandType::AssignMaterial: {
            // TODO: Implement material assignment when material system supports it
            // auto* mat_cmd = static_cast<AssignMaterialCommand*>(cmd);
            // context->set_entity_material(cmd->entity_id, mat_cmd->material_id);
            break;
        }
        
        case CommandType::SetTrailParams: {
            auto* trail_cmd = static_cast<SetTrailParamsCommand*>(cmd);
            context->set_entity_trail(cmd->entity_id, trail_cmd->max_points);
            break;
        }
        
        case CommandType::SetEntityColor: {
            auto* color_cmd = static_cast<SetEntityColorCommand*>(cmd);
            context->set_entity_color(cmd->entity_id, color_cmd->r, color_cmd->g, color_cmd->b, color_cmd->a);
            break;
        }
        
        case CommandType::SetEntityVisible: {
            auto* vis_cmd = static_cast<SetEntityVisibleCommand*>(cmd);
            context->set_entity_renderable(cmd->entity_id, vis_cmd->visible);
            break;
        }
        
        case CommandType::ApplySnapshot: {
            auto* snap_cmd = static_cast<ApplySnapshotCommand*>(cmd);
            context->apply_entity_snapshot(cmd->entity_id, snap_cmd->pos_x, snap_cmd->pos_y, snap_cmd->pos_z);
            break;
        }
    }
}

} // namespace astraeus

#endif // ASTRAEUS_COMMAND_BUFFER_HPP
