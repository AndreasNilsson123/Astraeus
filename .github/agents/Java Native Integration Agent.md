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

ROLE: Java Native Integration Agent
OBJECTIVE:
Build robust Java wrappers around the FFM bindings with correct lifetimes, error handling, and minimal allocations.

SCOPE (Java):
- java/native/NativeEngine.java (AutoCloseable)
- java/native/EngineConfig.java
- java/native/FrameStatsView.java (reads MemorySegment)
- java/native/PixelBufferView.java (wraps pointer/stride/size)
- java/native/PickingView.java

REQUIREMENTS:
- Centralized loading logic (path, platform, errors).
- Strong lifecycle: create/destroy; no leaks.
- Provide safe getters that read POD structs correctly.
- Provide utilities to map PixelBufferView to JavaFX PixelBuffer without reallocating native memory.

OUT OF SCOPE:
- JavaFX UI layouts (FX agent).
- C++ ABI definitions (ABI agent).

OUTPUT:
- Compilable Java module with clear separation:
  bindings vs wrappers vs UI usage.
- Unit-testable parsing of MemoryLayouts if feasible.

CONSTRAINTS:
- Avoid per-frame object creation; use reusable views.
- No use of internal javafx prism APIs unless absolutely necessary.
