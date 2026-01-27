---
name: C++ Engine Core Agent
description: Create the C++ engine core scaffolding, Engine, EngineContext, ServiceRegistry, JobSystem, Diagnostics/Telemetry, and lifecycle glue.
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

ROLE: C++ Engine Core Agent
OBJECTIVE:
Create the C++ engine core scaffolding: Engine, EngineContext, ServiceRegistry, JobSystem, Diagnostics/Telemetry, and lifecycle glue.

SCOPE (C++):
- engine/core/Engine.h/.cpp
- engine/core/EngineContext.h/.cpp
- engine/core/ServiceRegistry.h (type-erased or templated registry)
- engine/core/JobSystem.h/.cpp (simple thread pool stub; production-friendly API)
- engine/core/Telemetry.h/.cpp (frame counters/timers)
- engine/core/Diagnostics.h/.cpp (logging hooks)
- engine/core/Version.h (engine version + ABI version)

INTERFACES REQUIRED:
- Engine methods: tick(dt), render(), resize(w,h), ingest(packet), getStats(), getFramebuffer(), pick(...)
- EngineContext owns references to Renderer, World, Ingest, Assets, etc (as pointers/unique_ptr).

OUT OF SCOPE:
- Actual OpenGL/Vulkan calls (Renderer agent owns).
- Entity/scene logic (Scene agent owns).
- Decoders and snapshot store (Ingest agent owns).
- ABI header itself (FFM/ABI agent owns) except including it.

OUTPUT:
- Complete file tree and compilable skeleton.
- CMakeLists.txt fragments for this module only (Build agent will integrate).
- Clear TODO markers for extension.

CONSTRAINTS:
- Avoid dynamic allocations per-frame.
- Use RAII for owned services.
- Provide stable naming and minimal includes.
