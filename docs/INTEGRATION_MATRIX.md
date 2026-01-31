# Astraeus Engine↔Java Integration Matrix

**Version:** 0.1.0  
**Date:** 2026-01-31  
**Purpose:** Comprehensive feature coverage matrix mapping C API → Java bindings → Java wrappers → AstraeusApp UI usage

This document provides the authoritative mapping of all engine capabilities across the FFM boundary, ensuring complete feature coverage and identifying gaps.

---

## Table of Contents

- [Integration Status Summary](#integration-status-summary)
- [Feature Coverage Matrix](#feature-coverage-matrix)
  - [Engine Lifecycle](#engine-lifecycle)
  - [Rendering & Frame Management](#rendering--frame-management)
  - [Viewport Management](#viewport-management)
  - [Camera Management](#camera-management)
  - [Material System](#material-system)
  - [Scene & Entity Management](#scene--entity-management)
  - [Picking & Selection](#picking--selection)
  - [Data Ingestion](#data-ingestion)
  - [Telemetry & Diagnostics](#telemetry--diagnostics)
- [Gap Analysis](#gap-analysis)
- [Implementation Priority](#implementation-priority)

---

## Integration Status Summary

**Status Legend:**
- ✅ **Working** - Fully implemented and tested, used in AstraeusApp
- 🟡 **Stubbed** - Binding exists, wrapper exists, but minimal/no UI usage
- 🔴 **Missing** - C API exists but Java binding missing or incomplete
- 🟢 **Complete** - C API → Java binding → Wrapper → UI integration all working

| Layer | Total | Working | Stubbed | Missing |
|-------|-------|---------|---------|---------|
| **C API Functions** | 41 | 36 | 5 | 0 |
| **Java Bindings** | 36 | 36 | 0 | 5 |
| **Java Wrappers** | 30+ | 25 | 5 | 0 |
| **UI Integration** | 25+ | 15 | 10 | 0 |

---

## Feature Coverage Matrix

### Engine Lifecycle

| Feature | C API Symbol | Java Binding | Java Wrapper | AstraeusApp Usage | Status |
|---------|-------------|--------------|--------------|-------------------|--------|
| **API Version** | `astraeus_api_version()` | `API_VERSION` | `NativeEngine.getApiVersion()` | Console log on startup | 🟢 Complete |
| **Error Handling** | `astraeus_last_error()` | `LAST_ERROR` | `NativeEngine.getLastError()` | Error dialogs | 🟢 Complete |
| **Engine Creation** | `astraeus_create_engine()` | `CREATE_ENGINE` | `NativeEngine(width, height, validation)` | "Initialize Engine" button | 🟢 Complete |
| **Engine Destruction** | `astraeus_destroy_engine()` | `DESTROY_ENGINE` | `NativeEngine.close()` | Application shutdown | 🟢 Complete |
| **Engine Validation** | `astraeus_is_valid()` | `IS_VALID` | `NativeEngine.isValid()` | State checks throughout app | 🟢 Complete |

**Notes:**
- All lifecycle functions fully integrated and working
- Error handling used consistently across UI
- AutoCloseable pattern ensures proper cleanup

---

### Rendering & Frame Management

| Feature | C API Symbol | Java Binding | Java Wrapper | AstraeusApp Usage | Status |
|---------|-------------|--------------|--------------|-------------------|--------|
| **Begin Frame** | `astraeus_begin_frame()` | `BEGIN_FRAME` | `NativeEngine.beginFrame(dt)` | ViewportPane render loop | 🟢 Complete |
| **End Frame** | `astraeus_end_frame()` | `END_FRAME` | `NativeEngine.endFrame()` | ViewportPane render loop | 🟢 Complete |
| **Resize Viewport** | `astraeus_resize_viewport()` | `RESIZE_VIEWPORT` | `NativeEngine.resizeViewport(w,h)` | ViewportPane size change handlers | 🟢 Complete |
| **Configure Readback** | `astraeus_configure_readback()` | `CONFIGURE_READBACK` | `NativeEngine.configureReadback()` | ViewportPane initialization | 🟢 Complete |
| **Get Color Buffer** | `astraeus_get_color_buffer()` | `GET_COLOR_BUFFER` | `NativeEngine.getColorBuffer()` | FxViewport pixel readback | 🟢 Complete |
| **Get ID Buffer** | `astraeus_get_id_buffer()` | `GET_ID_BUFFER` | `NativeEngine.getIdBuffer()` | PickingView for entity selection | 🟢 Complete |
| **Get Frame Stats** | `astraeus_get_frame_stats()` | *(missing)* | *(N/A)* | *(N/A)* | 🔴 Missing |

**Notes:**
- Core rendering loop fully functional with double-buffered readback
- Frame stats binding missing but telemetry stats are used instead
- Readback stability critical for JavaFX integration (pointer must remain stable)

**Gap:** `astraeus_get_frame_stats()` not bound - need to add binding or rely on telemetry stats

---

### Viewport Management

| Feature | C API Symbol | Java Binding | Java Wrapper | AstraeusApp Usage | Status |
|---------|-------------|--------------|--------------|-------------------|--------|
| **Create Viewport** | `astraeus_viewport_create()` | `VIEWPORT_CREATE` | `NativeEngine.createViewport()` | ViewportPane initialization | 🟢 Complete |
| **Destroy Viewport** | `astraeus_viewport_destroy()` | `VIEWPORT_DESTROY` | `NativeViewport.close()` | ViewportPane cleanup | 🟢 Complete |
| **Resize Viewport** | `astraeus_viewport_resize()` | `VIEWPORT_RESIZE` | `NativeViewport.resize()` | ViewportPane size handlers | 🟢 Complete |
| **Get Viewport Color** | `astraeus_viewport_get_color()` | `VIEWPORT_GET_COLOR` | `NativeViewport.getColorBuffer()` | FxViewport display | 🟢 Complete |
| **Get Viewport ID Buffer** | `astraeus_viewport_get_idbuffer()` | `VIEWPORT_GET_IDBUFFER` | `NativeViewport.getIdBuffer()` | PickingView selection | 🟢 Complete |

**Notes:**
- Multi-viewport architecture ready but AstraeusApp uses single viewport currently
- All viewport operations working correctly
- Viewport handle lifecycle managed via AutoCloseable

---

### Camera Management

| Feature | C API Symbol | Java Binding | Java Wrapper | AstraeusApp Usage | Status |
|---------|-------------|--------------|--------------|-------------------|--------|
| **Get Active Camera** | `astraeus_camera_get_active()` | `CAMERA_GET_ACTIVE` | `NativeViewport.getActiveCamera()` | ViewportPane camera controls | 🟢 Complete |
| **Get Camera Desc** | `astraeus_camera_get_desc()` | `CAMERA_GET_DESC` | `NativeCamera.getDesc()` | Camera info display (minimal) | 🟡 Stubbed |
| **Set Camera Desc** | `astraeus_camera_set_desc()` | `CAMERA_SET_DESC` | `NativeCamera.setDesc()` | Mouse/keyboard camera controls | 🟢 Complete |
| **Destroy Camera** | `astraeus_camera_destroy()` | *(missing)* | *(N/A)* | *(N/A)* | 🔴 Missing |
| **Legacy Set Camera** | `astraeus_set_camera()` | *(missing)* | *(N/A)* | *(N/A)* | 🔴 Missing |
| **Legacy Set Projection** | `astraeus_set_camera_projection()` | *(missing)* | *(N/A)* | *(N/A)* | 🔴 Missing |

**Notes:**
- New camera API (get_active, get_desc, set_desc) working well
- Legacy camera functions not bound (intentional - superseded by descriptor API)
- Camera mode switching (orbit/fly/pan) working via setDesc
- Camera info display in UI is minimal - could be enhanced

**Gaps:**
- `astraeus_camera_destroy()` not bound (low priority - cameras are lightweight handles)
- Legacy camera functions not bound (intentional deprecation)

---

### Material System

| Feature | C API Symbol | Java Binding | Java Wrapper | AstraeusApp Usage | Status |
|---------|-------------|--------------|--------------|-------------------|--------|
| **Create Material** | `astraeus_material_create()` | `MATERIAL_CREATE` | `NativeEngine.createMaterial()` | *(minimal)* | 🟡 Stubbed |
| **Update Material** | `astraeus_material_update()` | `MATERIAL_UPDATE` | `NativeMaterial.update()` | *(minimal)* | 🟡 Stubbed |
| **Destroy Material** | `astraeus_material_destroy()` | `MATERIAL_DESTROY` | `NativeMaterial.close()` | *(minimal)* | 🟡 Stubbed |
| **Set Entity Material** | `astraeus_entity_set_material()` | `ENTITY_SET_MATERIAL` | *(not wrapped)* | *(not used)* | 🟡 Stubbed |

**Notes:**
- Material API fully bound and wrapped but minimal UI integration
- PBR material properties supported: base color, metallic, roughness, alpha mode
- Texture ID support exists but no texture loading in UI yet
- Entity material assignment binding exists but not wrapped or used

**Gaps:**
- No material editor UI panel
- No material library/palette
- Entity-material assignment not exposed in wrapper
- No texture loading or management

---

### Scene & Entity Management

| Feature | C API Symbol | Java Binding | Java Wrapper | AstraeusApp Usage | Status |
|---------|-------------|--------------|--------------|-------------------|--------|
| **Create Entity** | `astraeus_create_entity()` | `CREATE_ENTITY` | `NativeEngine.createEntity()` | "Create Entity" button, batch creation | 🟢 Complete |
| **Destroy Entity** | `astraeus_destroy_entity()` | `DESTROY_ENTITY` | `NativeEngine.destroyEntity()` | "Clear All" button, entity deletion | 🟢 Complete |
| **Set Transform** | `astraeus_set_entity_transform()` | `SET_ENTITY_TRANSFORM` | `NativeEngine.setEntityTransform()` | SceneManager.syncTransformToEngine() | 🟢 Complete |
| **Set Renderable** | `astraeus_set_entity_renderable()` | `SET_ENTITY_RENDERABLE` | `NativeEngine.setEntityRenderable()` | Entity visibility toggle | 🟢 Complete |
| **Set Color** | `astraeus_set_entity_color()` | `SET_ENTITY_COLOR` | `NativeEngine.setEntityColor()` | SceneManager.syncColorToEngine() | 🟢 Complete |
| **Set Trail** | `astraeus_set_entity_trail()` | `SET_ENTITY_TRAIL` | `NativeEngine.setEntityTrail()` | *(not used)* | 🟡 Stubbed |
| **Apply Snapshot** | `astraeus_apply_entity_snapshot()` | *(missing)* | *(N/A)* | *(N/A)* | 🔴 Missing |

**Notes:**
- Core entity CRUD operations working perfectly
- Transform and color manipulation via SceneManager abstraction
- Trail rendering capability exists but not exposed in UI
- Snapshot API (for WorldSync ingestion) not bound yet

**Gaps:**
- `astraeus_apply_entity_snapshot()` not bound - needed for data ingestion workflow
- Trail feature not used in UI - could add trail toggle in entity inspector
- No entity parenting/hierarchy (future feature)

---

### Picking & Selection

| Feature | C API Symbol | Java Binding | Java Wrapper | AstraeusApp Usage | Status |
|---------|-------------|--------------|--------------|-------------------|--------|
| **Pick Entity** | `astraeus_pick()` | `PICK` | `NativeEngine.pick(x, y)` | ViewportPane mouse click handler | 🟢 Complete |
| **Pick Result** | PickResult struct | PICK_RESULT_LAYOUT | PickResult record | Entity selection in SceneOutliner | 🟢 Complete |

**Notes:**
- ID buffer-based picking fully working
- PickResult includes entity ID, depth, world position
- Click-to-select integrated with scene outliner and inspector
- Highlight selected entity working

---

### Data Ingestion

| Feature | C API Symbol | Java Binding | Java Wrapper | AstraeusApp Usage | Status |
|---------|-------------|--------------|--------------|-------------------|--------|
| **Ingest Data** | `astraeus_ingest_data()` | *(missing)* | *(N/A)* | *(N/A)* | 🔴 Missing |

**Notes:**
- C++ ingest manager exists but not exposed via C API
- No Java bindings for data ingestion
- No UI for loading simulation snapshots

**Gaps:**
- `astraeus_ingest_data()` needs binding
- Need decoder registration mechanism
- Need schema definition for common formats (JSON, binary)
- Need UI file picker and format selector
- Need progress indication for large data loads

---

### Telemetry & Diagnostics

| Feature | C API Symbol | Java Binding | Java Wrapper | AstraeusApp Usage | Status |
|---------|-------------|--------------|--------------|-------------------|--------|
| **Enable Telemetry** | `astraeus_enable_telemetry()` | `ENABLE_TELEMETRY` | `NativeEngine.enableTelemetry()` | Telemetry toggle in UI | 🟢 Complete |
| **Is Telemetry Enabled** | `astraeus_is_telemetry_enabled()` | `IS_TELEMETRY_ENABLED` | `NativeEngine.isTelemetryEnabled()` | Status checks | 🟢 Complete |
| **Get Frame Stats** | `astraeus_get_telemetry_frame_stats()` | `GET_TELEMETRY_FRAME_STATS` | `NativeEngine.getTelemetryStats()` | TelemetryPane display | 🟢 Complete |
| **Get History** | `astraeus_get_telemetry_history()` | `GET_TELEMETRY_HISTORY` | `NativeEngine.getTelemetryHistory()` | TimelinePane charts | 🟢 Complete |
| **Get Pass Count** | `astraeus_get_pass_count()` | `GET_PASS_COUNT` | `NativeEngine.getPassCount()` | TelemetryPane pass breakdown | 🟢 Complete |
| **Get Pass Timing** | `astraeus_get_pass_timing()` | `GET_PASS_TIMING` | `NativeEngine.getPassTiming()` | TelemetryPane pass breakdown | 🟢 Complete |

**Notes:**
- Comprehensive telemetry system fully integrated
- Real-time FPS, frame time, CPU/GPU time displayed
- Pass-by-pass timing breakdown working
- Historical timeline charts functional
- Zero-overhead when disabled

---

## Gap Analysis

### Critical Gaps (Blocking Core Features)

1. **Data Ingestion Pipeline** (🔴 High Priority)
   - C API: `astraeus_ingest_data()` exists but not bound
   - Missing: Java binding, wrapper method, UI file picker
   - Impact: Cannot load external simulation data
   - Owner: Data Ingest & Sync Agent

2. **Entity Snapshot Application** (🔴 High Priority)
   - C API: `astraeus_apply_entity_snapshot()` not bound
   - Missing: Java binding, wrapper method
   - Impact: Cannot use WorldSync for time-series data
   - Owner: Data Ingest & Sync Agent

### Medium Priority Gaps

3. **Material UI Integration** (🟡 Medium Priority)
   - C API: Material functions fully bound ✅
   - Wrapper: NativeMaterial wrapper exists ✅
   - Missing: Material editor panel, texture loading, entity-material assignment wrapper
   - Impact: Cannot edit material properties visually
   - Owner: JavaFX Visualization Agent

4. **Frame Stats (non-Telemetry)** (🟡 Low Priority)
   - C API: `astraeus_get_frame_stats()` not bound
   - Workaround: Telemetry stats used instead ✅
   - Impact: Minor - telemetry provides better metrics anyway
   - Owner: FFM Agent (if needed)

5. **Trail Rendering UI** (🟡 Low Priority)
   - C API: `astraeus_set_entity_trail()` fully bound ✅
   - Wrapper: `NativeEngine.setEntityTrail()` exists ✅
   - Missing: UI toggle in entity inspector
   - Impact: Feature not discoverable
   - Owner: JavaFX Visualization Agent

### Intentional Non-Bindings

The following C API functions are **intentionally not bound** (superseded or deprecated):

- `astraeus_set_camera()` - Legacy, use `astraeus_camera_set_desc()` instead
- `astraeus_set_camera_projection()` - Legacy, use `astraeus_camera_set_desc()` instead
- `astraeus_camera_destroy()` - Low priority, cameras are lightweight handles

---

## Implementation Priority

### Phase 1: Data Ingestion (Critical)
**Owner:** Data Ingest & Sync Agent

1. Bind `astraeus_ingest_data()` in EngineBindings.java
2. Wrap in NativeEngine.ingestData()
3. Bind `astraeus_apply_entity_snapshot()` in EngineBindings.java
4. Wrap in NativeEngine.applyEntitySnapshot()
5. Create IngestPane UI:
   - File picker for simulation data
   - Format selector (JSON, binary, custom)
   - Schema validator display
   - Progress indicator
6. Test with sample data files

**Deliverable:** End-to-end data ingestion from file → engine → visualization

---

### Phase 2: Material System UI (Medium)
**Owner:** JavaFX Visualization Agent

1. Wrap `astraeus_entity_set_material()` in NativeEngine
2. Create MaterialEditorPane UI:
   - Color pickers for base color
   - Sliders for metallic, roughness
   - Alpha mode selector
   - Texture slots (placeholder for now)
3. Create MaterialLibraryPane:
   - List of materials
   - Add/delete/duplicate materials
   - Assign to selected entity
4. Integrate with entity inspector
5. Test material changes on entities

**Deliverable:** Visual material editing in UI

---

### Phase 3: Trail Rendering UI (Low)
**Owner:** JavaFX Visualization Agent

1. Add trail toggle checkbox in SceneInspectorPane
2. Add max trail points spinner
3. Wire up to `NativeEngine.setEntityTrail()`
4. Add trail color override (future)
5. Test with moving entities

**Deliverable:** Trail rendering visible and controllable

---

### Phase 4: Multi-Viewport UI (Future)
**Owner:** JavaFX Visualization Agent

1. Add "New Viewport" button in toolbar
2. Create viewport tabs or split panes
3. Wire up multiple ViewportPane instances
4. Test independent camera controls per viewport
5. Test performance with multiple viewports

**Deliverable:** Multiple simultaneous viewports

---

## Maintenance Guidelines

### Updating This Matrix

When adding/modifying features:

1. **C API Change:**
   - Update EngineAPI.h
   - Regenerate ABI structs if needed
   - Add row to matrix with "🔴 Missing" status in Java Binding column

2. **Java Binding Added:**
   - Add binding to EngineBindings.java
   - Update matrix: Java Binding column → binding name, status → "🟡 Stubbed"

3. **Java Wrapper Added:**
   - Add method to NativeEngine/NativeViewport/etc.
   - Update matrix: Java Wrapper column → method name, status remains "🟡 Stubbed"

4. **UI Integration:**
   - Add usage in AstraeusApp or tool panes
   - Update matrix: AstraeusApp Usage column → description, status → "🟢 Complete"

### Backlog Sync

- Keep [ROADMAP.md](ROADMAP.md) in sync with Gap Analysis section
- Reference specific matrix rows in backlog items
- Mark items as complete when status changes to "🟢 Complete"

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 0.1.0 | 2026-01-31 | Initial matrix with 41 C API functions mapped |

