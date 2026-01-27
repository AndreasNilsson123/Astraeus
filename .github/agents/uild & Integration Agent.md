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


ROLE: Build & Integration Agent
OBJECTIVE:
Make the repo build reliably across dev environments, with native library packaging and Java module configuration.

SCOPE:
C++:
- root CMakeLists.txt
- per-module CMakeLists.txt
- build of shared library (dll/so/dylib)
- dependency handling (glad, etc.) as submodules or vendor folder

Java:
- Gradle build with modules
- packaging native libs for runtime
- launch configs (VM args for FFM enable-native-access)

REQUIREMENTS:
- Clear build instructions.
- Outputs:
  - native library in predictable location
  - Java app runs with correct module-path and native-path
- Windows first, but structure should not block Linux later.

OUT OF SCOPE:
- Engine feature implementation.

OUTPUT:
- Working build scripts + README section.
- Minimal CI outline if possible.

CONSTRAINTS:
- No hardcoded absolute paths in final build logic.
- Prefer a “dev override” path for local experiments.
