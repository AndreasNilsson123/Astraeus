#ifndef ASTRAEUS_RENDER_GRAPH_HPP
#define ASTRAEUS_RENDER_GRAPH_HPP

#include <cstdint>
#include <memory>
#include <vector>

namespace astraeus {

class RenderDevice;
class World;
class RenderPass;
class TelemetrySystem;

/**
 * Render graph manages the execution of multiple render passes.
 * Passes can include: grid, tracks, volumes, symbols, overlays, etc.
 */
class RenderGraph {
public:
    RenderGraph(RenderDevice* device, World* world, TelemetrySystem* telemetry);
    ~RenderGraph();

    bool initialize();
    void shutdown();

    /**
     * Execute all render passes.
     */
    void execute();

    /**
     * Handle viewport resize.
     */
    void on_resize(uint32_t width, uint32_t height);

    /**
     * Add a render pass to the graph.
     */
    void add_pass(std::unique_ptr<RenderPass> pass);

private:
    RenderDevice* device_;
    World* world_;
    TelemetrySystem* telemetry_;
    std::vector<std::unique_ptr<RenderPass>> passes_;
    bool is_initialized_;
};

/**
 * Base class for all render passes.
 */
class RenderPass {
public:
    virtual ~RenderPass() = default;

    virtual bool initialize(RenderDevice* device) = 0;
    virtual void execute(RenderDevice* device, World* world) = 0;
    virtual void on_resize(uint32_t width, uint32_t height) = 0;

    /**
     * Get the name of this render pass (for telemetry).
     */
    virtual const char* get_name() const = 0;

protected:
    RenderPass() = default;
};

} // namespace astraeus

#endif // ASTRAEUS_RENDER_GRAPH_HPP
