#include "RenderGraph.hpp"
#include "RenderDevice.hpp"
#include "../scene/World.hpp"
#include "../core/Telemetry.hpp"
#include <iostream>

namespace astraeus {

RenderGraph::RenderGraph(RenderDevice* device, World* world, TelemetrySystem* telemetry)
    : device_(device)
    , world_(world)
    , telemetry_(telemetry)
    , is_initialized_(false)
{
}

RenderGraph::~RenderGraph() {
    shutdown();
}

bool RenderGraph::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[RenderGraph] Initializing render graph" << std::endl;

    // TODO: Initialize default passes (grid, tracks, etc.)
    // For now, no passes added

    is_initialized_ = true;
    return true;
}

void RenderGraph::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[RenderGraph] Shutting down" << std::endl;
    passes_.clear();
    is_initialized_ = false;
}

void RenderGraph::execute() {
    // Execute all passes in order, with telemetry timing
    for (auto& pass : passes_) {
        // Begin pass timing
        if (telemetry_) {
            telemetry_->begin_pass(pass->get_name());
        }
        
        // Execute the pass
        pass->execute(device_, world_);
        
        // End pass timing
        if (telemetry_) {
            telemetry_->end_pass();
        }
    }
}

void RenderGraph::on_resize(uint32_t width, uint32_t height) {
    for (auto& pass : passes_) {
        pass->on_resize(width, height);
    }
}

void RenderGraph::add_pass(std::unique_ptr<RenderPass> pass) {
    if (pass && pass->initialize(device_)) {
        passes_.push_back(std::move(pass));
    }
}

} // namespace astraeus
