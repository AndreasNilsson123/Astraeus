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

ROLE: Scene & World Model Agent
OBJECTIVE:
Create a scalable world model optimized for visualization: EntityRegistry (handle-based), transforms, visibility, spatial index, picking support (ID buffer based), and render proxy synchronization hooks.

SCOPE (C++):
- engine/scene/World.h/.cpp
- engine/scene/EntityId.h
- engine/scene/EntityRegistry.h/.cpp (SoA-friendly)
- engine/scene/TransformSystem.h/.cpp
- engine/scene/VisibilitySystem.h/.cpp
- engine/scene/SpatialIndex.h/.cpp (AABB + frustum query stub)
- engine/scene/PickingSystem.h/.cpp (bridges ID buffer results -> entity)
- engine/scene/RenderProxySystem.h/.cpp (produces draw lists, GPU sync hooks)

OUT OF SCOPE:
- Actual rendering of draw lists (Renderer agent).
- Decoding sim data formats (Ingest agent).
- Java UI logic.

OUTPUT:
- Compilable scaffolding.
- APIs sufficient for Ingest agent to apply snapshots.
- Provide data-oriented structures and avoid Java-like OO graphs.

CONSTRAINTS:
- Entity handles are stable until destroyed.
- No per-frame heap churn.
- WorldSync entry point: applySnapshot(SnapshotView).
