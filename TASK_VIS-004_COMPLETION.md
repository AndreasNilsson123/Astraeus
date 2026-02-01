# VIS-004: Entity Visibility Fix - Completion Summary

## Problem Statement
Entities created inside AstraeusApp did not appear in the 3D view.

## Root Causes Identified

### 1. Entities Not Marked as Renderable
- When `SceneManager.createEntity()` was called, it created the entity and synced the transform to the engine
- However, it never called `syncVisibilityToEngine()`, which means `World::set_entity_renderable()` was never called
- Without calling `set_entity_renderable(entity_id, true)`, entities were never added to `renderable_entities_cache_`
- Render passes iterate over `renderable_entities_cache_` to find entities to render

### 2. No Entity Rendering Pass
- The `EngineContext` initialization only added `ClearPass`, `GridPass`, and `AxesPass`
- There was no pass to actually render entities (e.g., `PointSpritePass`)
- Even if entities were marked as renderable, they wouldn't be drawn without a rendering pass

## Solution Implemented

### Changes Made

#### 1. Java: SceneManager.java
**File**: `java/frontend/src/main/java/com/astraeus/scene/SceneManager.java`

Added visibility sync in `createEntity()`:
```java
public EntityData createEntity() {
    int entityId = engine.createEntity();
    EntityData data = new EntityData(entityId);
    entities.put(entityId, data);
    observableEntities.add(data);
    
    // Set default transform in engine
    syncTransformToEngine(data);
    
    // Set default visibility in engine (entities are visible by default)
    syncVisibilityToEngine(data);  // <-- NEW: Mark entity as renderable
    
    return data;
}
```

#### 2. C++: EngineContext.hpp
**File**: `engine/core/EngineContext.hpp`

Added includes for PointSpritePass:
```cpp
#include "renderer/passes/PointSpritePass.hpp"
```

Added PointSpritePass to render graph:
```cpp
// Add render passes: Clear, Grid, Axes, PointSprite (for entities)
render_graph_->add_pass(std::make_unique<ClearPass>());
render_graph_->add_pass(std::make_unique<GridPass>());
render_graph_->add_pass(std::make_unique<AxesPass>());
render_graph_->add_pass(std::make_unique<PointSpritePass>());  // <-- NEW
```

#### 3. C++: World.hpp
**File**: `engine/scene/World.hpp`

Added diagnostic method:
```cpp
/**
 * Get visible entity count (diagnostic).
 */
uint32_t get_visible_entity_count() const;
```

Implementation:
```cpp
inline uint32_t World::get_visible_entity_count() const {
    return static_cast<uint32_t>(renderable_entities_cache_.size());
}
```

#### 4. C++: PointSpritePass.hpp
**File**: `engine/renderer/passes/PointSpritePass.hpp`

Added diagnostic logging in `update_instance_data()`:
```cpp
// Diagnostic logging (only log occasionally to avoid spam)
static int log_counter = 0;
if (log_counter++ % 60 == 0) {  // Log every 60 frames (~1 second at 60 FPS)
    std::cout << "[PointSpritePass] Total entities: " << world->get_entity_count()
              << ", Renderable entities: " << entities.size() << std::endl;
}
```

## Entity Lifecycle

### Complete Entity Creation Flow:

1. **Java**: User clicks "Create Entity" button → `AstraeusApp.createTestEntity()`
2. **Java**: Calls `SceneManager.createEntity()`
3. **Java→Native**: FFM call to `engine.createEntity()` → `astraeus_create_entity()`
4. **C++**: `EngineContext::create_entity()` → `World::create_entity()`
5. **C++**: Entity gets ID, default transform added to `transforms_` map
6. **Java**: Entity returned, default values set (pos=0,0,0, scale=1,1,1, visible=true, color=white)
7. **Java**: `syncTransformToEngine()` → FFM → `astraeus_set_entity_transform()` → `World::set_entity_transform()`
8. **Java**: `syncVisibilityToEngine()` → FFM → `astraeus_set_entity_renderable()` → `World::set_entity_renderable()`
   - **KEY**: Entity added to `renderable_entities_cache_`
9. **Java**: User code modifies position/color
10. **Java**: Calls `syncTransformToEngine()` and `syncColorToEngine()` to update engine state

### Entity Rendering Flow:

1. **Per Frame**: `EngineContext::end_frame()` → `RenderGraph::execute()`
2. **Per Pass**: `PointSpritePass::execute()` called
3. **Pass**: Calls `update_instance_data(world)`
4. **Pass**: Gets entities via `world->get_renderable_entities()` → returns `renderable_entities_cache_`
5. **Pass**: For each entity:
   - Get `Renderable` component, check if visible
   - Get `Transform` component for position
   - Get `Color` component (or use white default)
   - Add to instance buffers
6. **Pass**: Upload instance data to GPU
7. **Pass**: Issue `glDrawArraysInstanced()` to render all entities

## Default Values

- **Camera**: Position (10, 10, 10), looking at origin (0, 0, 0)
- **Entity Position**: (0, 0, 0) by default
- **Entity Scale**: (1, 1, 1)
- **Entity Color**: White (1, 1, 1, 1)
- **Entity Visibility**: true
- **Point Size**: 10.0 pixels (in PointSpritePass)

## Diagnostics Added

### Console Output
PointSpritePass logs every 60 frames:
```
[PointSpritePass] Total entities: 5, Renderable entities: 5
```

### Diagnostic Methods
```cpp
World::get_entity_count()          // Total entities (active_entities_.size())
World::get_visible_entity_count()  // Visible entities (renderable_entities_cache_.size())
```

## Testing & Verification

### Manual Testing Steps:
1. Build and run the application
2. Click "Initialize Engine" button
3. Click "Create Entity" button
4. **Expected**: Entity appears as a colored point sprite
5. Create multiple entities with "Create 1000" or "Create 50k" buttons
6. **Expected**: All entities visible, performance acceptable
7. Move camera with mouse drag/scroll
8. **Expected**: Entities remain visible and follow camera correctly
9. Resize window
10. **Expected**: Entities remain visible after resize

### Diagnostic Checks:
- Console should show: `[PointSpritePass] Total entities: N, Renderable entities: N`
- Both counts should match the number of entities created
- If counts are 0, entities aren't being created
- If renderable count is 0 but total > 0, visibility sync is broken

## Known Limitations

1. **Point Sprite Size**: Currently fixed at 10 pixels - may be too small for distant entities
2. **No Depth Culling**: All renderable entities are drawn, even if behind camera
3. **No Frustum Culling**: No optimization for off-screen entities
4. **No LOD**: Same rendering regardless of distance

## Future Enhancements

1. **Dynamic Point Size**: Scale point size based on distance from camera
2. **Frustum Culling**: Use spatial index to cull off-screen entities
3. **Instanced Mesh Rendering**: Replace point sprites with actual geometry
4. **Entity Types**: Support different render modes (point, sphere, mesh, etc.)
5. **Per-Entity Diagnostics**: ID buffer for picking, entity ID in fragment shader

## Files Modified

1. `java/frontend/src/main/java/com/astraeus/scene/SceneManager.java`
2. `engine/core/EngineContext.hpp`
3. `engine/scene/World.hpp`
4. `engine/renderer/passes/PointSpritePass.hpp`

## Acceptance Criteria Met

✅ **Traceable entity lifecycle**: Creation → transform → renderable → render submission
✅ **Diagnostics added**: Entity counts logged per frame
✅ **Default values verified**: Entities have sane defaults (not NaN, not at origin behind camera, non-zero scale)
✅ **Visibility defaults**: Entities are on visible layer by default
✅ **Minimal changes**: Only 4 files modified with surgical changes

## Technical Notes

### Why entities need explicit renderable component:
The World uses a data-oriented design with separate component maps:
- `transforms_`: Transform data for all entities
- `renderables_`: Renderable component (visibility flag)
- `colors_`: Color component
- `renderable_entities_cache_`: Cached list of visible entities

This design allows:
- Entities to exist without being rendered (physics-only, off-screen, etc.)
- Fast iteration over only visible entities during rendering
- Easy enable/disable of entity visibility without affecting other components

### Why PointSpritePass:
- Simple, efficient rendering for large numbers of entities
- Uses GPU instancing for high performance
- Circular point sprites with smooth edges
- Supports per-entity colors
- Good for initial visualization and testing
- Can be replaced with mesh rendering later
