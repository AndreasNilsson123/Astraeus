# PostChain Framework Usage Guide

## Overview

The PostChain framework provides a flexible, extensible post-processing pipeline for the Astraeus renderer. It supports tone mapping, gamma correction, and provides hooks for future effects like bloom and FXAA.

## Architecture

```
Main Rendering → PostChain → Final Output
                    ↓
    [ToneMap] → [Gamma] → [Bloom*] → [FXAA*]
                                    (* = stub/future)
```

## Basic Usage

### 1. Enable PostChain

```cpp
// Get the render graph from engine context
RenderGraph* render_graph = engine_context->get_render_graph();

// Enable post-processing
render_graph->set_post_chain_enabled(true);
```

### 2. Configure Post-Processing Passes

```cpp
// Get the post-chain
PostChain* post_chain = render_graph->get_post_chain();
if (post_chain) {
    // Add tone mapping pass
    auto tone_map = std::make_unique<ToneMappingPass>();
    tone_map->set_operator(ToneMappingPass::ToneMapOperator::ACES);
    tone_map->set_exposure(1.0f);
    post_chain->add_pass(std::move(tone_map));
    
    // Add gamma correction pass
    auto gamma = std::make_unique<GammaCorrectionPass>();
    gamma->set_gamma(2.2f); // sRGB
    post_chain->add_pass(std::move(gamma));
}
```

### 3. Enable/Disable Individual Passes

```cpp
PostChain* post_chain = render_graph->get_post_chain();
if (post_chain) {
    // Get specific pass by index
    PostProcessPass* tone_map = post_chain->get_pass(0);
    if (tone_map) {
        tone_map->set_enabled(false); // Disable tone mapping
    }
}
```

## Available Passes

### ToneMappingPass

Applies tone mapping to convert HDR values to displayable range.

**Operators:**
- `None` - Pass-through (no tone mapping)
- `Reinhard` - Simple Reinhard tone mapping
- `ReinhardLum` - Reinhard with luminance preservation
- `ACES` - ACES filmic tone mapping (recommended)

**Configuration:**
```cpp
auto tone_map = std::make_unique<ToneMappingPass>();
tone_map->set_operator(ToneMappingPass::ToneMapOperator::ACES);
tone_map->set_exposure(1.5f);  // Increase exposure
```

### GammaCorrectionPass

Applies gamma correction for color space conversion.

**Configuration:**
```cpp
auto gamma = std::make_unique<GammaCorrectionPass>();
gamma->set_gamma(2.2f);  // Standard sRGB gamma
```

### BloomPass (Stub)

Future extension point for bloom post-processing.

**Planned Features:**
- Bright pixel extraction
- Gaussian blur with ping-pong
- Configurable intensity and threshold

**Configuration (for future use):**
```cpp
auto bloom = std::make_unique<BloomPass>();
bloom->set_threshold(1.0f);       // Brightness threshold
bloom->set_intensity(0.5f);       // Bloom strength
bloom->set_blur_iterations(5);    // Blur quality
bloom->set_enabled(true);         // Enable when implemented
```

### FXAAPass (Stub)

Future extension point for Fast Approximate Anti-Aliasing.

**Planned Features:**
- Edge detection
- Sub-pixel anti-aliasing
- Quality presets

**Configuration (for future use):**
```cpp
auto fxaa = std::make_unique<FXAAPass>();
fxaa->set_quality(FXAAPass::Quality::High);
fxaa->set_edge_threshold(0.166f);
fxaa->set_enabled(true);  // Enable when implemented
```

## Advanced Usage

### Dynamic Pass Ordering

```cpp
PostChain* post_chain = render_graph->get_post_chain();

// Clear existing passes
post_chain->clear_passes();

// Add in custom order
post_chain->add_pass(std::make_unique<ToneMappingPass>());
post_chain->add_pass(std::make_unique<BloomPass>());      // When implemented
post_chain->add_pass(std::make_unique<GammaCorrectionPass>());
post_chain->add_pass(std::make_unique<FXAAPass>());       // When implemented
```

### Runtime Enable/Disable

```cpp
// Disable entire post-chain
post_chain->set_enabled(false);

// Or disable specific passes
for (size_t i = 0; i < post_chain->get_pass_count(); ++i) {
    PostProcessPass* pass = post_chain->get_pass(i);
    if (pass && pass->get_name() == std::string("ToneMapping")) {
        pass->set_enabled(false);
    }
}
```

## Performance Considerations

1. **No Per-Frame Allocations**: PostChain pre-allocates framebuffers and reuses them
2. **Lazy Initialization**: PostChain is only created when first enabled
3. **Pass Overhead**: Each pass requires one full-screen quad draw
4. **Memory**: Two intermediate framebuffers (ping-pong) at viewport resolution

## Compatibility

- **JavaFX Readback**: ✓ Compatible (same RGBA8 format)
- **C ABI**: ✓ No changes required
- **Existing Rendering**: ✓ No impact when disabled (default state)

## Future Enhancements

To implement full post-processing integration:

1. Uncomment the PostChain execution code in `RenderGraph::execute()`
2. Pass `gl_device->get_color_texture()` as input
3. Pass `gl_device->get_main_fbo()` as output
4. Test readback compatibility with JavaFX viewport

```cpp
// In RenderGraph::execute() - future integration:
if (post_chain_enabled_ && post_chain_ && post_chain_->is_enabled()) {
    GLRenderDevice* gl_device = dynamic_cast<GLRenderDevice*>(device_);
    if (gl_device) {
        uint32_t color_texture = gl_device->get_color_texture();
        uint32_t main_fbo = gl_device->get_main_fbo();
        post_chain_->apply(color_texture, main_fbo);
    }
}
```

## Example: Complete Setup

```cpp
#include "renderer/RenderGraph.hpp"
#include "renderer/passes/post/PostChain.hpp"
#include "renderer/passes/post/ToneMappingPass.hpp"
#include "renderer/passes/post/GammaCorrectionPass.hpp"

void setup_post_processing(EngineContext* engine) {
    RenderGraph* render_graph = engine->get_render_graph();
    
    // Enable post-processing
    render_graph->set_post_chain_enabled(true);
    
    PostChain* post_chain = render_graph->get_post_chain();
    if (!post_chain) {
        std::cerr << "Failed to initialize PostChain" << std::endl;
        return;
    }
    
    // Configure tone mapping
    auto tone_map = std::make_unique<ToneMappingPass>();
    tone_map->set_operator(ToneMappingPass::ToneMapOperator::ACES);
    tone_map->set_exposure(1.0f);
    post_chain->add_pass(std::move(tone_map));
    
    // Configure gamma correction
    auto gamma = std::make_unique<GammaCorrectionPass>();
    gamma->set_gamma(2.2f);
    post_chain->add_pass(std::move(gamma));
    
    std::cout << "Post-processing configured with " 
              << post_chain->get_pass_count() << " passes" << std::endl;
}
```

## Troubleshooting

**PostChain not being created?**
- Ensure `set_post_chain_enabled(true)` is called
- Check that RenderDevice is initialized

**Passes not visible?**
- Currently, PostChain is infrastructure-only (not yet connected to rendering)
- Full integration requires uncommenting code in `RenderGraph::execute()`

**Performance issues?**
- Disable unused passes: `pass->set_enabled(false)`
- Reduce blur iterations in BloomPass (when implemented)
- Use lower FXAA quality (when implemented)
