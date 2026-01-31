# Astraeus Roadmap

**Version:** 0.1.0  
**Date:** 2026-01-31  
**Purpose:** Consolidated backlog and development roadmap for Astraeus visualization engine

This document provides a prioritized, de-duplicated backlog derived from the [Integration Matrix](INTEGRATION_MATRIX.md). All tasks reference specific matrix gaps to ensure traceability.

---

## Table of Contents

- [Project Status](#project-status)
- [Upcoming Milestones](#upcoming-milestones)
- [Phase 1: Data Ingestion (Critical)](#phase-1-data-ingestion-critical)
- [Phase 2: Material System UI](#phase-2-material-system-ui)
- [Phase 3: Enhanced Visualization](#phase-3-enhanced-visualization)
- [Phase 4: Advanced Features](#phase-4-advanced-features)
- [Phase 5: Performance & Scale](#phase-5-performance--scale)
- [Long-Term Vision](#long-term-vision)
- [Completed Work](#completed-work)

---

## Project Status

**Current State:** MVP with core rendering, scene management, and telemetry working

### What's Working ✅

- ✅ C++ engine core with OpenGL renderer
- ✅ Java FFM bindings (36/41 C API functions bound)
- ✅ JavaFX application with professional workspace layout
- ✅ Entity creation, transformation, coloring
- ✅ ID buffer-based picking and selection
- ✅ Camera controls (orbit, fly, pan modes)
- ✅ Comprehensive telemetry system with UI
- ✅ Scene outliner and entity inspector
- ✅ Stable pixel readback for JavaFX integration
- ✅ Multi-viewport architecture (single viewport in UI)

### What's Missing 🔴

- 🔴 Data ingestion pipeline (critical blocker)
- 🔴 Material editor UI
- 🔴 Texture loading and management
- 🔴 Trail rendering UI controls
- 🔴 Advanced lighting (point lights, shadows)
- 🔴 Asset loading (glTF, OBJ, FBX)
- 🔴 Mesh rendering (only basic cubes currently)

---

## Upcoming Milestones

### Milestone 1: Data Ingestion (Q1 2026)
**Goal:** Load and visualize external simulation data

**Key Deliverables:**
- Ingest data from JSON/binary files
- Apply time-series snapshots to entities
- File picker UI with format selector
- Sample data files for testing

**Depends On:** Phase 1 tasks

---

### Milestone 2: Visual Material Editing (Q1 2026)
**Goal:** Artists can create and edit materials visually

**Key Deliverables:**
- Material editor panel with PBR controls
- Material library/palette
- Assign materials to entities in inspector
- Live preview in viewport

**Depends On:** Phase 2 tasks

---

### Milestone 3: Advanced Rendering (Q2 2026)
**Goal:** Production-quality rendering with lighting and shadows

**Key Deliverables:**
- Point lights with attenuation
- Directional lights with shadow mapping
- Skybox/environment maps
- Post-processing effects (bloom, SSAO)

**Depends On:** Renderer & RenderGraph Agent work

---

### Milestone 4: Asset Pipeline (Q2 2026)
**Goal:** Load and render real 3D models

**Key Deliverables:**
- glTF 2.0 loader with materials
- OBJ loader (simple geometries)
- Mesh rendering with LOD
- Asset browser UI

**Depends On:** Asset pipeline C++ and Java integration

---

## Phase 1: Data Ingestion (Critical)

**Priority:** 🔴 **P0 - Critical**  
**Owner:** Data Ingest & Sync Agent  
**Status:** Not Started  
**Matrix Reference:** [Data Ingestion](INTEGRATION_MATRIX.md#data-ingestion), [Scene & Entity Management](INTEGRATION_MATRIX.md#scene--entity-management)

### Tasks

#### 1.1 Bind Ingest Data API
- [ ] Add `astraeus_ingest_data()` binding to EngineBindings.java
  - Function descriptor with (data pointer, size, format)
  - Method handle initialization
- [ ] Wrap in `NativeEngine.ingestData(byte[] data, int format)`
  - Allocate native memory segment
  - Copy Java byte array to native memory
  - Invoke C API
  - Error handling
- [ ] Test with mock data

**Acceptance Criteria:**
- Can call `engine.ingestData()` from Java
- C++ IngestManager receives data correctly
- Error cases handled gracefully

---

#### 1.2 Bind Entity Snapshot API
- [ ] Add `astraeus_apply_entity_snapshot()` binding to EngineBindings.java
  - Function descriptor with (entity_id, pos_x, pos_y, pos_z)
  - Method handle initialization
- [ ] Wrap in `NativeEngine.applyEntitySnapshot(int entityId, float x, float y, float z)`
- [ ] Test with moving entities

**Acceptance Criteria:**
- Can apply position snapshots to entities
- Trail history updated correctly (if trails enabled)
- Performance acceptable for high-frequency updates

---

#### 1.3 Create IngestPane UI
- [ ] New `com.astraeus.tools.IngestPane` class
  - File picker button (FileChooser)
  - Format selector combo box (JSON, Binary, Custom)
  - Load button with progress indicator
  - Status text area for validation errors
- [ ] Integrate IngestPane into WorkspaceWindow
  - Add as left panel tab or tool menu item
  - Wire up to engine instance
- [ ] File format validation
  - Check file extension
  - Validate JSON schema (if JSON)
  - Display parse errors
- [ ] Progress indication for large files
  - Use Task<Void> for async loading
  - Show progress bar
  - Allow cancellation

**Acceptance Criteria:**
- User can pick file from filesystem
- File is validated before loading
- Progress shown for large files
- Errors displayed in UI
- Entities appear in viewport after load

---

#### 1.4 Sample Data Files
- [ ] Create `assets/samples/simulation_data/`
  - `orbit_data.json` - 2-body orbit simulation
  - `nbody_data.json` - N-body gravitational simulation
  - `trajectory.bin` - Binary trajectory format (custom)
- [ ] Document data format schema
  - JSON schema for orbit/nbody formats
  - Binary format specification
  - Example decoder in C++
- [ ] README with usage instructions

**Acceptance Criteria:**
- At least 2 sample data files
- Loads successfully via IngestPane
- Visualizes correctly in viewport
- Documentation complete

---

#### 1.5 Time-Series Playback (Stretch)
- [ ] Timeline scrubber UI
  - Slider for time position
  - Play/pause/stop buttons
  - Speed control (1x, 2x, 5x, 10x)
- [ ] Frame interpolation
  - Smooth transitions between snapshots
  - Linear interpolation for positions
- [ ] Loop mode
  - Checkbox to loop playback
  - Reset to start on complete

**Acceptance Criteria:**
- Can scrub through loaded time-series
- Playback is smooth
- Performance acceptable for 1000+ frames

---

## Phase 2: Material System UI

**Priority:** 🟡 **P1 - Medium**  
**Owner:** JavaFX Visualization Agent  
**Status:** Not Started  
**Matrix Reference:** [Material System](INTEGRATION_MATRIX.md#material-system)

### Tasks

#### 2.1 Wrap Entity-Material Assignment
- [ ] Add `NativeEngine.setEntityMaterial(int entityId, NativeMaterial material)`
  - Invoke `ENTITY_SET_MATERIAL` binding
  - Pass material handle
  - Error handling
- [ ] Test assigning materials to entities

**Acceptance Criteria:**
- Can assign material to entity from Java
- Material properties visible on entity in viewport
- Changing material updates entity appearance

---

#### 2.2 Create MaterialEditorPane UI
- [ ] New `com.astraeus.tools.MaterialEditorPane` class
  - **Base Color Section:**
    - ColorPicker for base color RGB
    - Slider for alpha (0-1)
  - **PBR Properties Section:**
    - Slider for metallic (0-1)
    - Slider for roughness (0-1)
  - **Alpha Mode Section:**
    - Radio buttons: Opaque / Blend / Mask
  - **Texture Section (Placeholder):**
    - Labels for "Base Color Texture: None"
    - Labels for "Normal Texture: None"
    - Future: Add texture picker buttons
  - **Preview Section:**
    - Show material swatch (future: small 3D preview)
- [ ] Wire up to `NativeMaterial.update()`
  - Create MaterialDesc from UI values
  - Invoke update on material
  - Update preview
- [ ] Real-time preview
  - Apply changes immediately on slider drag
  - Or "Apply" button if performance is concern

**Acceptance Criteria:**
- Material properties editable via sliders and pickers
- Changes reflected in viewport on entities using material
- UI responsive, no lag

---

#### 2.3 Create MaterialLibraryPane UI
- [ ] New `com.astraeus.tools.MaterialLibraryPane` class
  - ListView of materials in scene
  - "New Material" button
  - "Duplicate Material" button
  - "Delete Material" button
  - Material preview thumbnails (optional)
- [ ] Material selection
  - Click material to edit in MaterialEditorPane
  - Highlight selected material
- [ ] Material naming
  - Editable name field
  - Unique name validation
- [ ] Persistence (stretch)
  - Save materials to JSON file
  - Load materials from JSON file

**Acceptance Criteria:**
- Can create/delete/duplicate materials
- Materials listed with names
- Selection syncs with MaterialEditorPane
- Materials persist across sessions (stretch)

---

#### 2.4 Integrate with Entity Inspector
- [ ] Add "Material" section to SceneInspectorPane
  - Combo box showing available materials
  - "Create New Material" button
  - Preview of assigned material
- [ ] Wire up material assignment
  - Selecting material assigns to entity
  - Entity updates immediately in viewport
- [ ] Bulk assignment (stretch)
  - Multi-select entities in outliner
  - Assign material to all selected

**Acceptance Criteria:**
- Can assign material to entity via inspector
- Material changes reflected immediately
- Bulk assignment works (stretch)

---

## Phase 3: Enhanced Visualization

**Priority:** 🟡 **P2 - Medium**  
**Owner:** Renderer & RenderGraph Agent (C++), JavaFX Visualization Agent (UI)  
**Status:** Not Started  
**Matrix Reference:** [Rendering & Frame Management](INTEGRATION_MATRIX.md#rendering--frame-management)

### Tasks

#### 3.1 Trail Rendering UI Controls
- [ ] Add "Trails" section to SceneInspectorPane
  - Checkbox: "Enable Trail"
  - Spinner: "Max Trail Points" (10-1000)
  - Color picker: "Trail Color" (optional override)
- [ ] Wire up to `NativeEngine.setEntityTrail()`
  - Pass max_points from spinner
  - Disable trail when checkbox unchecked (max_points = 0)
- [ ] Test with moving entities
  - Apply entity snapshots to see trail
  - Adjust max points and see trail length change

**Acceptance Criteria:**
- Trail toggle visible in inspector
- Trail renders correctly in viewport
- Max points configurable
- Performance acceptable with trails enabled

---

#### 3.2 Lighting System
**Owner:** Renderer & RenderGraph Agent

- [ ] Point lights
  - Add C API: `astraeus_create_point_light(pos, color, intensity, radius)`
  - Bind in Java
  - Wrap in NativeLightSource class
  - UI: LightEditorPane with position/color/intensity controls
- [ ] Directional lights
  - Add C API: `astraeus_create_directional_light(dir, color, intensity)`
  - Bind in Java
  - Wrap in NativeLightSource class
- [ ] Shadow mapping (stretch)
  - Shadow map render pass
  - PCF filtering
  - UI: Shadow toggle and quality settings

**Acceptance Criteria:**
- At least 8 point lights supported
- Lighting visible on entities
- Performance acceptable (60 FPS with 8 lights)

---

#### 3.3 Environment and Atmosphere
**Owner:** Renderer & RenderGraph Agent

- [ ] Skybox rendering
  - Cubemap texture support
  - Skybox render pass
  - C API for skybox texture upload
  - Java binding and UI file picker
- [ ] Procedural sky (stretch)
  - Time-of-day parameter
  - Sun position
  - Atmospheric scattering approximation

**Acceptance Criteria:**
- Can load and display skybox cubemap
- Sky renders behind scene
- Procedural sky looks realistic (stretch)

---

## Phase 4: Advanced Features

**Priority:** 🟢 **P3 - Low**  
**Status:** Future

### Tasks

#### 4.1 Multi-Viewport UI
- [ ] Add "New Viewport" button in toolbar
- [ ] Create split pane or tab-based layout
- [ ] Wire up multiple ViewportPane instances
  - Each viewport has own NativeViewport handle
  - Each viewport has own camera
- [ ] Test independent camera controls
- [ ] Test performance with 2-4 viewports

**Acceptance Criteria:**
- Can create multiple viewports
- Each viewport independently controllable
- Performance acceptable (30+ FPS per viewport)

---

#### 4.2 glTF Asset Loading
**Owner:** Asset Pipeline C++ Agent, Java Native Integration Agent

- [ ] glTF 2.0 parser in C++
  - Parse JSON structure
  - Load binary buffers
  - Extract meshes, materials, textures
- [ ] C API for asset loading
  - `astraeus_load_asset(path, out_asset_handle)`
  - `astraeus_create_entity_from_asset(asset_handle)`
- [ ] Java bindings and wrapper
  - `NativeEngine.loadAsset(String path)`
  - Returns `NativeAsset` handle
- [ ] AssetBrowserPane UI
  - File browser for .gltf/.glb files
  - Thumbnail preview
  - "Import" button
  - Drag-and-drop into viewport

**Acceptance Criteria:**
- Can load glTF 2.0 files
- Meshes render correctly with materials
- Textures apply correctly
- UI for browsing and importing assets

---

#### 4.3 Advanced Camera Controls
- [ ] Camera bookmarks
  - Save camera positions
  - List of bookmarks in UI
  - Jump to bookmark on click
- [ ] Smooth camera interpolation
  - Animate camera position changes
  - Configurable speed
- [ ] Focus on entity
  - Right-click entity → "Focus Camera"
  - Camera moves to frame entity

**Acceptance Criteria:**
- Can save and load camera bookmarks
- Camera animation smooth
- Focus on entity works correctly

---

## Phase 5: Performance & Scale

**Priority:** 🟢 **P3 - Low**  
**Status:** Future

### Tasks

#### 5.1 Instanced Rendering
**Owner:** Renderer & RenderGraph Agent

- [ ] Instance buffer for entity transforms
  - SoA layout for transforms
  - Upload to GPU
  - Instanced draw calls
- [ ] Test with 100k+ entities
  - Performance profiling
  - Target: 60 FPS with 100k cubes

**Acceptance Criteria:**
- Instanced rendering working
- Performance scales to 100k entities

---

#### 5.2 Frustum Culling
**Owner:** Scene & World Model Agent

- [ ] Camera frustum extraction
- [ ] AABB bounding boxes for entities
- [ ] Frustum-AABB intersection test
- [ ] Cull entities outside frustum
- [ ] Telemetry for culled entity count

**Acceptance Criteria:**
- Entities outside view frustum not rendered
- Performance improvement measurable

---

#### 5.3 Level of Detail (LOD)
**Owner:** Scene & World Model Agent, Renderer & RenderGraph Agent

- [ ] LOD levels for meshes
  - High, medium, low poly versions
  - Distance-based LOD selection
- [ ] Automatic LOD generation (future)
  - Mesh simplification algorithm
  - Generate LOD levels on asset load

**Acceptance Criteria:**
- LOD transitions smooth
- Performance improvement measurable

---

## Long-Term Vision

### Year 1 Goals (2026)
- ✅ MVP rendering and UI (complete)
- 🔴 Data ingestion and time-series playback
- 🔴 Material system with PBR
- 🔴 Asset loading (glTF)
- 🔴 Advanced lighting and shadows

### Year 2 Goals (2027)
- Multi-threaded rendering (separate render thread)
- Vulkan backend for high performance
- Distributed rendering for large datasets
- VR/AR support
- Plugin system for custom render passes

### Year 3+ Goals (2028+)
- Python bindings for data ingestion scripting
- Jupyter notebook integration
- Cloud rendering for remote visualization
- Real-time collaboration (multiple users)

---

## Completed Work

**See [ARCHITECTURE.md](ARCHITECTURE.md) for full architecture details.**

### Completed Features ✅

- **Engine Core (C++):**
  - ✅ EngineContext with lifecycle management
  - ✅ RenderDevice abstraction (OpenGL backend)
  - ✅ RenderGraph with pluggable passes
  - ✅ Scene/World with entity-component system
  - ✅ Camera system (orbit, fly, pan modes)
  - ✅ Telemetry and diagnostics
  - ✅ Platform abstraction (Win32, Linux)
  - ✅ Stable readback buffers for JavaFX

- **FFM Integration (Java):**
  - ✅ EngineBindings with 36 C API function bindings
  - ✅ Auto-generated struct layouts
  - ✅ NativeEngine wrapper with AutoCloseable
  - ✅ NativeViewport, NativeCamera, NativeMaterial wrappers
  - ✅ PixelBufferView for zero-copy readback
  - ✅ PickResult for entity selection

- **JavaFX Application:**
  - ✅ AstraeusApp with professional workspace layout
  - ✅ ViewportPane with FxViewport rendering
  - ✅ SceneOutlinerPane with entity tree
  - ✅ SceneInspectorPane with entity properties
  - ✅ TelemetryPane with real-time metrics
  - ✅ TimelinePane with historical charts
  - ✅ ConsolePane for logging
  - ✅ WorkspaceWindow with docking-like panels

- **Render Passes:**
  - ✅ GridPass (ground plane grid)
  - ✅ MeshPass (entity rendering with colors)
  - ✅ PickingPass (ID buffer generation)
  - ✅ UIPass (future overlays)

- **Build & Integration:**
  - ✅ CMake build system for C++
  - ✅ Maven build for Java
  - ✅ Cross-platform support (Linux, Windows)
  - ✅ Regenerate ABI script for struct layouts

---

## Backlog Maintenance

### How to Update This Roadmap

1. **New Feature Request:**
   - Assess if it fits existing phase or needs new phase
   - Add task with clear acceptance criteria
   - Update integration matrix if new C API needed
   - Assign priority and owner

2. **Task Completion:**
   - Check off task checkbox
   - Update integration matrix status (🟡 → 🟢)
   - Move to "Completed Work" section if entire feature done
   - Update project status summary

3. **Priority Changes:**
   - Re-order phases if priorities shift
   - Update priority tags (P0, P1, P2, P3)
   - Communicate changes to team

4. **Milestone Updates:**
   - Update milestone dates if timeline shifts
   - Add new milestones as project evolves
   - Link milestones to specific phases

### Backlog Hygiene

- **Weekly:** Review phase priorities, adjust as needed
- **Monthly:** Update milestone progress
- **Quarterly:** Review long-term vision alignment
- **Continuous:** Update integration matrix with new bindings

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 0.1.0 | 2026-01-31 | Initial roadmap with 5 phases, de-duplicated from matrix |

