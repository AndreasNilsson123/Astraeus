# VIS-002: Viewport/Scissor Fix Implementation

## Overview
This document describes the fixes implemented to ensure viewport and scissor settings cover the full framebuffer, addressing the issue where camera transforms were affecting only the top half of the viewport.

## Changes Made

### 1. GLRenderDevice Initialization (`GLRenderDevice.hpp`)
**Location**: `initialize()` method

Added explicit OpenGL state initialization:
```cpp
// Set initial OpenGL state for rendering
glViewport(0, 0, width_, height_);
glDisable(GL_SCISSOR_TEST);
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LEQUAL);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

**Rationale**: Ensures OpenGL state is properly initialized at startup with full framebuffer coverage.

### 2. Frame Begin (`GLRenderDevice.hpp`)
**Location**: `begin_frame()` method

Added explicit scissor test disable:
```cpp
glBindFramebuffer(GL_FRAMEBUFFER, main_fbo_);
glViewport(0, 0, width_, height_);
glDisable(GL_SCISSOR_TEST);
```

**Rationale**: Ensures every frame starts with correct viewport and no scissor clipping, preventing any per-frame state corruption.

### 3. Readback Operations (`GLRenderDevice.hpp`)
**Location**: `end_frame()` method

Added GL_PACK_ALIGNMENT setting:
```cpp
// Set pack alignment for proper row pitch handling
glPixelStorei(GL_PACK_ALIGNMENT, 4);
```

**Rationale**: Ensures correct pixel packing during texture readback to PBOs, preventing stride issues.

### 4. Resize Operations (`GLRenderDevice.hpp`)
**Location**: `resize()` method

Added viewport update after framebuffer recreation:
```cpp
create_framebuffers();
glViewport(0, 0, width_, height_);
```

**Rationale**: Ensures viewport is immediately updated when framebuffer size changes.

### 5. Diagnostic Pass
**New File**: `engine/renderer/passes/DiagnosticPass.hpp`

Created a diagnostic pass that renders a fullscreen UV gradient:
- Red channel increases from left to right (U coordinate)
- Green channel increases from bottom to top (V coordinate)
- Blue channel is constant at 0.3
- Alpha channel is 0.5 for semi-transparency

**Usage**:
```cpp
// In RenderGraph initialization
auto diagnostic = std::make_unique<DiagnosticPass>();
render_graph_->add_pass(std::move(diagnostic));

// Enable for debugging
diagnostic->set_enabled(true);
```

**Rationale**: Provides a visual tool to verify full framebuffer coverage. If the gradient covers the entire viewport, the viewport/scissor settings are correct.

## Testing

### Visual Verification
1. Enable the DiagnosticPass
2. Run the application
3. Verify that:
   - The UV gradient covers the entire 3D pane
   - Red increases smoothly from left to right
   - Green increases smoothly from bottom to top
   - No half-frame or partial coverage issues

### Camera Verification
1. Disable the DiagnosticPass
2. Run the application with normal scene rendering
3. Verify that:
   - Camera transforms affect the entire viewport
   - No rendering is clipped to half the screen
   - Readback buffer contains correct full-frame image

## Technical Details

### Viewport vs Scissor
- **Viewport**: Defines the transformation from NDC to window coordinates
- **Scissor**: Defines a rectangular clipping region

When scissor test is enabled, rendering is clipped to the scissor rectangle. Since we don't need this feature for offscreen rendering, we explicitly disable it.

### GL_PACK_ALIGNMENT
Controls the alignment of pixel rows in memory when reading from OpenGL:
- Default value: 4 bytes
- Our setting: 4 bytes (explicit)
- For RGBA8 (4 bytes per pixel), alignment doesn't matter, but we set it explicitly for clarity

### Framebuffer Attachments
The main FBO has three attachments:
1. Color texture (GL_COLOR_ATTACHMENT0): RGBA8
2. ID texture (GL_COLOR_ATTACHMENT1): R32UI (for picking)
3. Depth-stencil texture: GL_DEPTH24_STENCIL8

All attachments are sized to match width_ × height_.

## Related Files
- `engine/renderer/opengl/GLRenderDevice.hpp`: Core OpenGL device implementation
- `engine/renderer/passes/DiagnosticPass.hpp`: Diagnostic visualization pass
- `engine/renderer/RenderGraph.cpp`: Render pass execution
- `engine/core/EngineContext.hpp`: High-level engine coordination

## References
- Issue: VIS-002 - Ensure viewport/scissor and readback copy cover full framebuffer
- OpenGL specification: Section 13.6 (Pixel Storage Modes)
- OpenGL specification: Section 17.3 (Scissor Test)
