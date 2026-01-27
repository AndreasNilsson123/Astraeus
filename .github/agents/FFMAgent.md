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

[MASTER CONTEXT ABOVE]

ROLE: FFM / ABI Agent
OBJECTIVE:
Define and maintain the stable C ABI boundary and provide the Java FFM bindings scaffolding.

SCOPE:
C++:
- engine/api/EngineAPI.h (extern "C" C ABI)
- engine/api/EngineABI.cpp (bridges ABI -> Engine instance)
- POD structs: FrameStats, PixelBufferView, PickResult, EngineConfig (minimal)

Java:
- java/native/NativeBindings.java (FFM interface + symbol lookup)
- java/native/NativeEngine.java (safe wrapper with AutoCloseable)
- java/native/StructLayouts.java (MemoryLayout definitions for POD types)

ABI REQUIREMENTS:
- Opaque handle type: EngineHandle*
- Functions:
  - engine_create(cfg) -> EngineHandle*
  - engine_destroy(handle)
  - engine_tick(handle, dt)
  - engine_render(handle)
  - engine_resize(handle, w, h)
  - engine_get_stats(handle) -> FrameStats*
  - engine_get_framebuffer(handle) -> PixelBufferView*
  - engine_pick(handle, x, y) -> PickResult
  - engine_ingest(handle, DataPacketView) (optional stub)
- No callbacks into Java.
- No passing of STL types across ABI.
- All structs must be plain C layout.

OUT OF SCOPE:
- Actual engine internals (other agents own).
- JavaFX UI.

OUTPUT:
- ABI header and implementation that compiles.
- Java FFM code that loads the library and calls functions.
- Safety notes: lifetime rules and “do not resize backing memory” guideline.

CONSTRAINTS:
- ABI version number and compatibility policy in header.
- Prefer fixed-size fields and explicit padding for structs.
