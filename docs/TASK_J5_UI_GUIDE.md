# J5 UI Component Visual Guide

## Workspace Layout

```
┌────────────────────────────────────────────────────────────────────────────┐
│ Menu Bar: File | Edit | View | Tools | Help                                │
├────────────────┬───────────────────────────────────┬───────────────────────┤
│                │                                   │                       │
│  Scene         │  Center Viewport Area             │  Right Panel (Tabs)   │
│  Outliner      │  (Render Output)                  │                       │
│  ┌──────────┐  │                                   │  ┌─────────────────┐  │
│  │ Entity 1 │  │  ┌─────────────────────────────┐  │  │ Entities        │  │
│  │ Entity 2 │  │  │                             │  │  │ Inspector ◄──┐  │  │
│  │ Entity 3 │  │  │    3D Visualization         │  │  │ Telemetry    │  │  │
│  │ Camera   │  │  │                             │  │  │ Timeline     │  │  │
│  └──────────┘  │  │                             │  │  │ Pick Insp. ◄─┼──┼─ NEW (J5)
│                │  │                             │  │  │ Ingest Prog. │  │  │
│                │  └─────────────────────────────┘  │  └──────────────┘  │  │
│                │                                   │                    │  │
├────────────────┴───────────────────────────────────┴────────────────────┘  │
│  Console / Logs                                                            │
└────────────────────────────────────────────────────────────────────────────┘
```

## New Components (J5)

### 1. Enhanced Telemetry Pane

```
┌─────────────────────────────────────┐
│ Performance Telemetry               │
├─────────────────────────────────────┤
│ ☑ Enable Telemetry                  │
│                                     │
│ Frame:      1234                    │
│ FPS:        59.8                    │
│ CPU Time:   15.32 ms                │
│ GPU Time:   N/A                     │
│ Total Time: 16.72 ms                │
│ Draw Calls: 142                     │
│ Triangles:  45,678                  │
│ Passes:     3                       │
├─────────────────────────────────────┤
│ Render Pass Breakdown               │
│ ┌─────────────────────────────────┐ │
│ │ Pass Name  │ Time(ms) │ %       │ │
│ ├────────────┼──────────┼─────────┤ │
│ │ MeshPass   │ 8.234    │ 52.1%   │ │
│ │ GridPass   │ 4.123    │ 26.1%   │ │
│ │ PickingP.  │ 3.456    │ 21.8%   │ │
│ └────────────┴──────────┴─────────┘ │
├─────────────────────────────────────┤
│ History Charts              ◄─ NEW  │
│ ┌─────────────────────────────────┐ │
│ │ FPS (frames per second)         │ │
│ │ ╭─╮                             │ │
│ │ │ ╰╮  ╭─╮                        │ │
│ │ │  ╰──╯ ╰╮                       │ │
│ │ ╰────────╰───────────────────── │ │
│ │ 60                            0 │ │
│ └─────────────────────────────────┘ │
│ ┌─────────────────────────────────┐ │
│ │ Frame Time (milliseconds)       │ │
│ │     ╭─╮                         │ │
│ │ ╭───╯ ╰──╮  ╭──╮                │ │
│ │ ╰────────╰──╯  ╰──────────────  │ │
│ │ 20ms                         0  │ │
│ └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### 2. Pick Inspector Pane (NEW)

```
┌─────────────────────────────────────┐
│ Pick Inspector                      │
├─────────────────────────────────────┤
│ Status: Entity Selected ✓           │
│                                     │
│ ┌─ Pick Information ──────────────┐ │
│ │ Entity ID:  42                  │ │
│ │ Depth:      0.7234              │ │
│ └─────────────────────────────────┘ │
│                                     │
│ ┌─ World Position ────────────────┐ │
│ │ X:    12.345                    │ │
│ │ Y:    -5.678                    │ │
│ │ Z:    20.123                    │ │
│ └─────────────────────────────────┘ │
│                                     │
│ ┌─ Viewport Context ──────────────┐ │
│ │ Screen X:   425                 │ │
│ │ Screen Y:   312                 │ │
│ └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### 3. Enhanced Inspector Pane

```
┌─────────────────────────────────────┐
│ Inspector                           │
├─────────────────────────────────────┤
│ Entity ID: 42                       │
│ Name: [Entity 42            ]       │
│                                     │
│ ┌─ Transform ─────────────────────┐ │
│ │ Position                        │ │
│ │ X: [10.00] Y: [5.00] Z: [0.00] │ │
│ │ Rotation (degrees)              │ │
│ │ X: [0.00]  Y: [45.0] Z: [0.00] │ │
│ │ Scale                           │ │
│ │ X: [1.00]  Y: [1.00] Z: [1.00] │ │
│ └─────────────────────────────────┘ │
│                                     │
│ ┌─ Rendering ─────────────────────┐ │
│ │ ☑ Visible                       │ │
│ │ Color: 1.0, 1.0, 1.0, 1.0       │ │
│ └─────────────────────────────────┘ │
│                                     │
│ ┌─ Trail ─────────────────────────┐ │  ◄─ NEW (J5)
│ │ ☑ Enable Trail                  │ │
│ │ Max Points: [100    ]           │ │
│ └─────────────────────────────────┘ │
│                                     │
│ ┌─ Material ──────────────────────┐ │  ◄─ NEW (J5)
│ │ Material assignment:            │ │
│ │ [Select material...       ▼]    │ │
│ │ [ Assign to Selected ]          │ │
│ │ Note: Requires wrapper API (J6) │ │
│ └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### 4. Ingest Progress Pane (NEW)

```
┌─────────────────────────────────────┐
│ Ingest Progress                     │
├─────────────────────────────────────┤
│ ┌─ Current Job ───────────────────┐ │
│ │ physics_data.bin                │ │
│ │ Status: Processing...           │ │
│ └─────────────────────────────────┘ │
│                                     │
│ ┌─ Progress ──────────────────────┐ │
│ │ ████████████░░░░░░░░░░░         │ │
│ │ 52%                             │ │
│ │ Items: 520 / 1000               │ │
│ └─────────────────────────────────┘ │
│                                     │
│ Note: Ingest functionality requires │
│ D2/J6 integration                   │
└─────────────────────────────────────┘

┌─ When Error Occurs ─────────────────┐
│ ┌─ Error ─────────────────────────┐ │
│ │ Failed to parse entity data at  │ │
│ │ byte offset 12345               │ │
│ └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

## User Workflows

### Workflow 1: View Performance Telemetry

1. User clicks **Telemetry** tab in right panel
2. Checks **Enable Telemetry** checkbox
3. Views real-time stats (FPS, frame time, draw calls)
4. Examines pass breakdown table (sorted by time)
5. Monitors history charts to see trends

**Result**: User can identify performance bottlenecks and see which passes are expensive.

### Workflow 2: Inspect Picking Results

1. User clicks **Pick Inspector** tab in right panel
2. Clicks on an entity in the viewport
3. Pick Inspector updates with:
   - Entity ID
   - Depth value
   - World position (X, Y, Z)
   - Screen coordinates where clicked
4. Status shows "Entity Selected ✓" in green

**Result**: User confirms picking is working and can see exact world coordinates.

### Workflow 3: Enable Entity Trail

1. User selects an entity in the scene outliner
2. Clicks **Inspector** tab in right panel
3. Checks **Enable Trail** checkbox
4. Adjusts **Max Points** spinner to desired value (e.g., 100)
5. Entity's trail appears in viewport as it moves

**Result**: Trail feature is discoverable and works end-to-end without code.

### Workflow 4: Assign Material (Future J6)

1. User selects an entity in the scene outliner
2. Clicks **Inspector** tab in right panel
3. Scrolls to **Material** section
4. Selects material from dropdown (populated by J6)
5. Clicks **Assign to Selected** button
6. Entity's material changes in viewport

**Result**: Material assignment is discoverable (UI present, awaiting J6 wrapper).

### Workflow 5: Monitor Ingestion Progress (Future D2/J6)

1. User starts an ingestion job (via menu or code)
2. Clicks **Ingest Progress** tab in right panel
3. Views:
   - Job name (e.g., "physics_data.bin")
   - Progress bar (52%)
   - Items processed (520 / 1000)
   - Status message ("Processing...")
4. If error occurs, error section appears with details

**Result**: Ingestion progress is visible without UI hitching (UI present, awaiting D2/J6).

## Technical Details

### Update Rates
- **Telemetry**: 10-30 Hz (throttled)
- **Pick Inspector**: On-demand (user click)
- **Trail Controls**: On-demand (user interaction)
- **Ingest Progress**: 1-10 Hz (controlled by wrapper)

### Memory Efficiency
- **FrameStatsHistory**: Fixed ring buffer (no allocations during update)
- **TelemetryPane**: Reuses TableView rows (no allocations per frame)
- **Charts**: Canvas-based (redraws only when data changes)

### Integration Points
- All panes use `NativeEngine` public API only
- No direct dependencies on native internals
- Ready for J6 (material wrapper) and D2/J6 (ingest status) integration

---

**Status**: ✅ UI Implementation Complete
**Next Steps**: Manual testing and J6/D2 integration
