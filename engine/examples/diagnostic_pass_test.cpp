/**
 * Example: Using DiagnosticPass to verify viewport coverage
 * 
 * This example demonstrates how to enable the DiagnosticPass to visually
 * verify that the viewport and scissor settings cover the full framebuffer.
 */

#include "core/EngineContext.hpp"
#include "renderer/passes/DiagnosticPass.hpp"
#include <iostream>

int main() {
    // Create engine context
    astraeus::EngineContext::Config config;
    config.initial_width = 1280;
    config.initial_height = 720;
    config.enable_validation = true;
    config.enable_debug_output = true;
    
    astraeus::EngineContext engine(config);
    
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }
    
    // Add diagnostic pass to render graph
    // The diagnostic pass renders a fullscreen UV gradient:
    // - Red channel: increases from left to right
    // - Green channel: increases from bottom to top
    // - Blue channel: constant at 0.3
    // - Alpha: 0.5 (semi-transparent overlay)
    auto diagnostic = std::make_unique<astraeus::DiagnosticPass>();
    
    // Get render graph and add the pass
    auto* render_graph = engine.get_render_graph();
    render_graph->add_pass(std::move(diagnostic));
    
    // The diagnostic pass is disabled by default
    // To enable it for debugging:
    // diagnostic->set_enabled(true);
    
    std::cout << "Diagnostic pass added to render graph" << std::endl;
    std::cout << "To enable: get the pass from render graph and call set_enabled(true)" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected visual result when enabled:" << std::endl;
    std::cout << "  - Full screen should show gradient" << std::endl;
    std::cout << "  - Left edge: black (R=0, G varies)" << std::endl;
    std::cout << "  - Right edge: red (R=1, G varies)" << std::endl;
    std::cout << "  - Bottom edge: black-to-red (R varies, G=0)" << std::endl;
    std::cout << "  - Top edge: cyan-to-yellow (R varies, G=1)" << std::endl;
    std::cout << std::endl;
    std::cout << "If only part of the screen shows gradient:" << std::endl;
    std::cout << "  -> Viewport or scissor settings are incorrect" << std::endl;
    
    // Run a few frames
    for (int i = 0; i < 10; ++i) {
        engine.begin_frame(1.0 / 60.0);  // 60 FPS
        engine.end_frame();
    }
    
    engine.shutdown();
    
    std::cout << "Diagnostic test completed" << std::endl;
    return 0;
}
