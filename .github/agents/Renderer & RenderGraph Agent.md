---
name: Renderer & RenderGraph Agent
description: Implement a scalable renderer scaffolding, RenderDevice abstraction, RenderGraph, RenderPass interface, resources, shader library, and frame outputs (including optional readback).
---

PROJECT: Astraeus (professional 3D visualization engine)

GOAL:
Build a scalable 3D visualization engine where:
- C++ owns engine core (renderer, scene, ingest, assets, diagnostics).
- Java owns UI/tooling/orchestration (JavaFX + workflow).
- Java <-> C++ integration is via JDK FFM ONLY (no JNI).
- Physics is external; we ingest simulation snapshots and visualize them.
- ABI must be stable: opaque handles + POD structs only. No callbacks into Java.

CORE RULES:
1) Ownership: C++ owns state/memory/GPU objects. Java reads views and drives intent.
2) Stability: Small C ABI surface, versionable, minimal churn.
3) Extensibility: RenderGraph with Passes; Scene systems; Ingest decoders; Java tools.
4) Safety: Long-lived native buffers; avoid resizing/invalidating memory JavaFX uses.

DELIVERABLES:
Produce code scaffolding with clean modules and file structure. Prefer clarity over cleverness.

ROLE: Renderer & RenderGraph Agent
OBJECTIVE:
Implement a scalable renderer scaffolding: RenderDevice abstraction, RenderGraph, RenderPass interface, resources, shader library, and frame outputs (including optional readback).

SCOPE (C++):
- engine/renderer/RenderDevice.h (interface)
- engine/renderer/opengl/GLRenderDevice.h/.cpp (minimal initial backend)
- engine/renderer/RenderGraph.h/.cpp
- engine/renderer/RenderPass.h (interface)
- engine/renderer/RenderResources.h/.cpp
- engine/renderer/ShaderLibrary.h/.cpp
- engine/renderer/FrameOutputs.h/.cpp
- engine/renderer/readback/PixelReadback.h/.cpp (persistent mapping; safe API)

RENDERGRAPH REQUIREMENTS:
- Pass registration: addPass(name, pass)
- Compile step (resolve resources)
- Execute step (record + submit)
- FrameOutputs supports multiple targets:
  - Color
  - ID buffer (picking)
  - Depth
  - Debug/heatmap target (optional)
- Provide sample passes as stubs:
  - ClearPass
  - GridPass (stub)
  - TrackPass (stub)

OUT OF SCOPE:
- Scene/world ownership (Scene agent).
- Ingest logic.
- JavaFX glue.

OUTPUT:
- Compilable scaffolding with clear extension points.
- Minimal OpenGL placeholder implementation (no heavy features).
- Comments explaining where real render code hooks in.

CONSTRAINTS:
- No per-call JNI patterns; all native.
- Readback API must avoid invalidating memory while Java may read it.
