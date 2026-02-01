# Testing Guide for VIS-004 Entity Visibility Fix

## Prerequisites
1. Build the C++ engine library
2. Build the Java frontend application

## Quick Test Procedure

### Test 1: Single Entity Creation
1. Launch AstraeusApp
2. Click "Initialize Engine" button
3. Click "Create Entity" button
4. **Expected**: A colored point should appear in the 3D view
5. **Success Criteria**: Entity is visible as a colored dot

### Test 2: Multiple Entity Creation
1. Click "Create 1000" button
2. **Expected**: 1000 entities appear in a grid pattern
3. **Success Criteria**: Entities are visible, performance is acceptable
4. Check console for diagnostic output: `[PointSpritePass] Total entities: 1000, Renderable entities: 1000`

### Test 3: Camera Movement
1. With entities visible, drag mouse to rotate camera
2. Scroll to zoom in/out
3. **Expected**: Entities remain visible and move correctly with camera
4. **Success Criteria**: Entities follow camera motion smoothly

### Test 4: Window Resize
1. Resize the application window
2. **Expected**: Entities remain visible and properly positioned
3. **Success Criteria**: No visual artifacts or disappearing entities

## Console Output to Verify

Expected log output (appears every ~1 second):
```
[PointSpritePass] Total entities: 1, Renderable entities: 1
```

After creating 1000 entities:
```
[PointSpritePass] Total entities: 1000, Renderable entities: 1000
```

If you see:
```
[PointSpritePass] Total entities: 1000, Renderable entities: 0
```
Then the visibility sync is broken.

## Debugging Tips

### Entities Not Visible?
1. Check console for `[PointSpritePass]` output
2. If no output: PointSpritePass not initialized
3. If "Renderable entities: 0": Visibility sync broken
4. If "Total entities: 0": Entity creation broken

### Build Issues?
1. Make sure OpenGL libraries are installed
2. Check that FFM bindings are generated
3. Verify C++ library is in Java library path

### Point Too Small?
Edit `engine/renderer/passes/PointSpritePass.hpp`:
```cpp
point_size_(10.0f)  // Increase to 20.0f or 30.0f for larger points
```

### Camera Can't See Entities?
Default camera is at (10, 10, 10) looking at origin.
Entities are created at random positions in range [-10, 10] for x, y, z.
If entities are behind camera, try:
- Creating more entities (some will be in front)
- Rotating camera to look around
- Modifying entity creation range in AstraeusApp.java

## Expected Behavior Summary

✅ **Create Entity**: Entity appears immediately as a colored point
✅ **Multiple Entities**: All entities visible, grid pattern for large counts
✅ **Camera Movement**: Entities follow camera smoothly
✅ **Window Resize**: Entities remain visible after resize
✅ **Console Output**: Diagnostic logs show entity counts
✅ **Performance**: 1000 entities at 60 FPS, 50k entities at 30+ FPS (hardware dependent)

## Known Limitations

1. **Fixed Point Size**: Points are 10 pixels regardless of distance
2. **No Culling**: All entities rendered even if off-screen
3. **No Picking**: Clicking entities not yet implemented
4. **Simple Rendering**: Point sprites only, no complex geometry

## Code Verification

### Key Changes to Verify:

1. **SceneManager.java line 40**:
   ```java
   syncVisibilityToEngine(data);
   ```

2. **EngineContext.hpp line 365**:
   ```cpp
   render_graph_->add_pass(std::make_unique<PointSpritePass>());
   ```

3. **World.hpp**: Check `get_visible_entity_count()` exists

4. **PointSpritePass.hpp**: Check diagnostic logging in `update_instance_data()`

If any of these are missing, the fix is incomplete.

## Performance Benchmarks

### Expected Frame Rates (on typical hardware):
- 100 entities: 60 FPS
- 1,000 entities: 60 FPS
- 10,000 entities: 45-60 FPS
- 50,000 entities: 30-45 FPS
- 100,000 entities: 15-30 FPS

GPU instancing keeps performance high even with many entities.

## Success Criteria

The fix is successful if:
1. ✅ Entities appear when created
2. ✅ Console shows matching entity/renderable counts
3. ✅ Entities remain visible during camera movement
4. ✅ Entities remain visible after window resize
5. ✅ Performance is acceptable for typical entity counts

If all criteria are met, VIS-004 is resolved.
