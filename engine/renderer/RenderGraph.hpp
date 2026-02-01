#ifndef ASTRAEUS_RENDER_GRAPH_HPP
#define ASTRAEUS_RENDER_GRAPH_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>

namespace astraeus {

class RenderDevice;
class GLRenderDevice;
class World;
class RenderPass;
class Telemetry;
class PostChain;

/**
 * Render graph manages the execution of multiple render passes.
 * Passes can include: grid, tracks, volumes, symbols, overlays, etc.
 */
class RenderGraph {
public:
    RenderGraph(RenderDevice* device, World* world, Telemetry* telemetry);
    ~RenderGraph();

    bool initialize();
    void shutdown();

    /**
     * Execute all render passes.
     */
    void execute();
    
    /**
     * Enable or disable post-processing chain.
     * When enabled, PostChain will be initialized on next execute if not already.
     */
    void set_post_chain_enabled(bool enabled);
    
    /**
     * Check if post-processing is enabled.
     */
    bool is_post_chain_enabled() const;

    /**
     * Handle viewport resize.
     */
    void on_resize(uint32_t width, uint32_t height);

    /**
     * Add a render pass to the graph.
     */
    void add_pass(std::unique_ptr<RenderPass> pass);

    /**
     * Get the post-processing chain.
     * @return Pointer to the PostChain, or nullptr if not initialized
     */
    PostChain* get_post_chain() const;

private:
    RenderDevice* device_;
    World* world_;
    Telemetry* telemetry_;
    std::vector<std::unique_ptr<RenderPass>> passes_;
    std::unique_ptr<PostChain> post_chain_;
    bool is_initialized_;
    bool post_chain_enabled_;
    
    /**
     * Initialize post-chain if not already initialized.
     */
    void ensure_post_chain_initialized();
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
    
    // Get a human-readable name for this pass (for telemetry)
    virtual const char* get_name() const = 0;

protected:
    RenderPass() = default;
};

} // namespace astraeus

#endif // ASTRAEUS_RENDER_GRAPH_HPP
