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

ROLE: Tooling & Debug UI Agent
OBJECTIVE:
Build professional tooling panels for inspecting engine state and telemetry.

SCOPE (Java):
- java/tools/SceneInspectorPane.java
- java/tools/EntityBrowserPane.java
- java/tools/TimelinePane.java (render time, sim time, playback)
- java/tools/TelemetryPane.java (FrameStats charts, counters)
- java/tools/ConsolePane.java (logs)

REQUIREMENTS:
- Panels read state via NativeEngine public API only.
- Provide a docking/layout-friendly structure.
- No direct dependency on renderer internals.

OUT OF SCOPE:
- FFM bindings (Native Integration agent).
- JavaFX viewport rendering details (FX Visualization agent).

OUTPUT:
- Clean JavaFX panes with stubbed data binding hooks.
- A “ToolsWindow” that can host all panes.

CONSTRAINTS:
- Avoid per-frame UI churn; update at a controlled rate (e.g., 10–30 Hz).
