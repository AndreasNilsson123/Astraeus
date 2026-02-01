# PostChain Integration Quick Reference

## Configuration

### Engine Initialization
```cpp
// Disable PostChain (default, backward compatible)
EngineContext::Config config;
config.enable_post_chain = false;
auto engine = std::make_unique<EngineContext>(config);

// Enable PostChain
config.enable_post_chain = true;
```

### Runtime Control
```cpp
// Enable/disable at runtime
engine->set_post_chain_enabled(true);
bool enabled = engine->is_post_chain_enabled();

// Access PostChain directly
RenderGraph* graph = engine->get_render_graph();
PostChain* chain = graph->get_post_chain();
```

## Default Pass Configuration

### Tone Mapping (Pass 0)
```cpp
auto* tone_map = dynamic_cast<ToneMappingPass*>(chain->get_pass(0));
tone_map->set_operator(ToneMappingPass::ToneMapOperator::ACES);
tone_map->set_exposure(1.2f);
```

### Gamma Correction (Pass 1)
```cpp
auto* gamma = dynamic_cast<GammaCorrectionPass*>(chain->get_pass(1));
gamma->set_gamma(2.2f);  // Standard sRGB
```

## Adding Custom Passes

```cpp
// Add bloom effect
auto bloom = std::make_unique<BloomPass>();
bloom->set_threshold(1.0f);
bloom->set_intensity(0.5f);
bloom->set_blur_iterations(4);
chain->add_pass(std::move(bloom));

// Add FXAA
auto fxaa = std::make_unique<FXAAPass>();
fxaa->set_quality(FXAAPass::Quality::High);
chain->add_pass(std::move(fxaa));
```

## Output Contract

| Property | Value | Notes |
|----------|-------|-------|
| Internal Format | GL_RGBA8 | Linear color space |
| Readback Format | PIXEL_FORMAT_BGRA8 | Converted by OpenGL |
| Gamma Correction | Once (2.2) | Only when enabled |
| Channel Order | BGRA | JavaFX compatible |
| Alpha Channel | Preserved | Through all passes |

## Performance

- **Disabled:** Zero overhead (early return)
- **Enabled:** ~0.5-1ms overhead @ 1080p
- **Memory:** ~16MB for intermediate buffers @ 1080p
- **Allocations:** None per-frame

## Debugging

### Disable PostChain
```cpp
// At config level
config.enable_post_chain = false;

// At runtime
engine->set_post_chain_enabled(false);

// At PostChain level
chain->set_enabled(false);

// At pass level
chain->get_pass(0)->set_enabled(false);
```

### Telemetry
```cpp
// Enable telemetry to see pass timings
engine->set_telemetry_enabled(true);
uint32_t pass_count = engine->get_telemetry_pass_count();
for (uint32_t i = 0; i < pass_count; ++i) {
    auto* timing = engine->get_telemetry_pass_timing(i);
    std::cout << timing->name << ": " << timing->duration_ms << "ms\n";
}
```

## Common Issues

### Output is too dark
```cpp
// Enable PostChain for gamma correction
engine->set_post_chain_enabled(true);
```

### Output is too bright (double gamma)
```cpp
// Ensure only one gamma correction
chain->get_pass(1)->set_enabled(false);
// Or check upstream rendering for gamma
```

### Channel swap (red/blue inverted)
```cpp
// Verify readback format is BGRA8 (default)
ReadbackConfig config;
config.format = PIXEL_FORMAT_BGRA8;
engine->configure_readback(&config, nullptr);
```

### Performance regression
```cpp
// Disable expensive passes
bloom->set_enabled(false);
fxaa->set_enabled(false);

// Or disable PostChain entirely
engine->set_post_chain_enabled(false);
```

## Integration with JavaFX

```java
// In Java viewport update
public void updateViewport() {
    PixelBufferView colorView = new PixelBufferView();
    viewport.getColor(colorView);
    
    // Create MemorySegment from stable pointer
    MemorySegment segment = MemorySegment.ofAddress(colorView.data)
        .reinterpret(colorView.stride * colorView.height);
    
    ByteBuffer buffer = segment.asByteBuffer();
    
    // Create WritableImage (BGRA format)
    PixelFormat<ByteBuffer> format = PixelFormat.getByteBgraPreInstance();
    PixelBuffer<ByteBuffer> pixelBuffer = new PixelBuffer<>(
        colorView.width, colorView.height, buffer, format);
    
    WritableImage image = new WritableImage(pixelBuffer);
    imageView.setImage(image);
}
```

## API Reference

### EngineContext
- `void set_post_chain_enabled(bool enabled)` - Enable/disable PostChain
- `bool is_post_chain_enabled() const` - Query state

### RenderGraph
- `void set_post_chain_enabled(bool enabled)` - Enable/disable PostChain
- `bool is_post_chain_enabled() const` - Query state
- `PostChain* get_post_chain() const` - Access PostChain

### PostChain
- `void set_enabled(bool enabled)` - Enable/disable chain
- `bool is_enabled() const` - Query state
- `void add_pass(std::unique_ptr<PostProcessPass> pass)` - Add pass
- `PostProcessPass* get_pass(size_t index)` - Get pass by index
- `size_t get_pass_count() const` - Get pass count
- `void apply(uint32_t input_texture, uint32_t output_fbo)` - Execute chain

### PostProcessPass (Base)
- `void set_enabled(bool enabled)` - Enable/disable pass
- `bool is_enabled() const` - Query state
- `const char* get_name() const` - Get pass name

### ToneMappingPass
- `void set_operator(ToneMapOperator op)` - Set tone map operator
- `void set_exposure(float exposure)` - Set exposure value
- Operators: `None`, `Reinhard`, `ReinhardLum`, `ACES`

### GammaCorrectionPass
- `void set_gamma(float gamma)` - Set gamma value (default: 2.2)

## See Also

- [TASK_E7_1_COMPLETION.md](TASK_E7_1_COMPLETION.md) - Full completion report
- [docs/TASK_E7_COMPLETION.md](docs/TASK_E7_COMPLETION.md) - Original framework
- [docs/POSTCHAIN_USAGE.md](docs/POSTCHAIN_USAGE.md) - Detailed usage guide
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) - System architecture
