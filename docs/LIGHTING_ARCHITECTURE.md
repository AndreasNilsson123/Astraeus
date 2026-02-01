# Lighting System v1 - Architecture Diagram

## Data Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                         Application Layer                        │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  │ Set lighting parameters
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                           World                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Light State:                                             │  │
│  │  • direction[3]     = {0.0f, -1.0f, 0.0f}               │  │
│  │  • color[3]         = {1.0f, 1.0f, 1.0f}                │  │
│  │  • intensity        = 1.0f                               │  │
│  │  • ambient[3]       = {0.2f, 0.2f, 0.2f}                │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                   │
│  API Methods:                                                    │
│  • set_light_direction(x, y, z)     [normalizes internally]    │
│  • set_light_color(r, g, b, intensity)                         │
│  • set_ambient_light(r, g, b)                                  │
│  • get_light_direction(...)                                    │
│  • get_light_color(...)                                        │
│  • get_ambient_light(...)                                      │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  │ Query light state
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                       LitMeshPass                                │
│                                                                   │
│  Execute Pipeline:                                               │
│  1. Get lighting parameters from World                          │
│  2. Create DirectionalLight struct                              │
│  3. Set light on LitMaterial                                    │
│  4. For each visible mesh:                                      │
│     • Compute MVP matrix                                        │
│     • Compute model matrix                                      │
│     • Compute normal matrix                                     │
│     • Set material parameters                                   │
│     • Draw mesh                                                 │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  │ Material binding & uniforms
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                       LitMaterial                                │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Shader Uniforms:                                         │  │
│  │  • uMVP              : mat4   (Model-View-Projection)    │  │
│  │  • uModel            : mat4   (Model matrix)             │  │
│  │  • uNormalMatrix     : mat3   (Normal transformation)    │  │
│  │  • uLightDirection   : vec3                              │  │
│  │  • uLightColor       : vec3                              │  │
│  │  • uLightIntensity   : float                             │  │
│  │  • uAmbientColor     : vec3                              │  │
│  │  • uViewPos          : vec3   (Camera position)          │  │
│  │  • uBaseColor        : vec4   (Material base color)      │  │
│  │  • uRoughness        : float                             │  │
│  │  • uSpecularStrength : float                             │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  │ Shader execution
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                      GPU - Vertex Shader                         │
│                                                                   │
│  Input:  aPosition (vec3), aNormal (vec3), aTexCoord (vec2)    │
│  Output: vWorldPos (vec3), vNormal (vec3), vTexCoord (vec2)    │
│                                                                   │
│  Transforms:                                                     │
│  • World position = uModel * vec4(aPosition, 1.0)              │
│  • World normal   = normalize(uNormalMatrix * aNormal)         │
│  • Clip position  = uMVP * vec4(aPosition, 1.0)                │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  │ Per-fragment interpolation
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                     GPU - Fragment Shader                        │
│                                                                   │
│  Lighting Calculation:                                          │
│                                                                   │
│  1. Normalize vectors:                                          │
│     normal   = normalize(vNormal)                               │
│     lightDir = normalize(-uLightDirection)                      │
│     viewDir  = normalize(uViewPos - vWorldPos)                  │
│                                                                   │
│  2. Ambient component:                                          │
│     ambient = uAmbientColor * baseColor.rgb                     │
│                                                                   │
│  3. Diffuse component (Lambert):                                │
│     diffuseStrength = max(dot(normal, lightDir), 0.0)          │
│     diffuse = diffuseStrength * uLightColor *                   │
│               uLightIntensity * baseColor.rgb                   │
│                                                                   │
│  4. Specular component (Blinn-Phong):                          │
│     halfwayDir = normalize(lightDir + viewDir)                  │
│     shininess = mix(2.0, 128.0, 1.0 - uRoughness)              │
│     specStrength = pow(max(dot(normal, halfwayDir), 0.0),      │
│                       shininess)                                 │
│     specular = uSpecularStrength * specStrength *               │
│                uLightColor * uLightIntensity                     │
│                                                                   │
│  5. Combine:                                                    │
│     finalColor = ambient + diffuse + specular                   │
│     FragColor = vec4(finalColor, baseColor.a)                  │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  │ Output to framebuffer
                                  ▼
                        ┌─────────────────┐
                        │  Final Render   │
                        │  (Lit 3D Scene) │
                        └─────────────────┘
```

## Component Relationships

```
MaterialLibrary
    │
    ├──> UnlitMaterial (existing)
    │       └──> Simple flat shading
    │
    └──> LitMaterial (new)
            ├──> DirectionalLight struct
            ├──> Lambert diffuse
            └──> Blinn-Phong specular
```

## Lighting Equation Breakdown

### Diffuse (Lambert)
```
diffuse = max(N · L, 0) × lightColor × intensity × baseColor

Where:
  N = surface normal (normalized)
  L = light direction (normalized, pointing towards light)
  · = dot product
```

**Visual Effect**: 
- Bright when surface faces light
- Dark when perpendicular
- Black when facing away

### Specular (Blinn-Phong)
```
specular = pow(max(N · H, 0), shininess) × specularStrength × lightColor × intensity

Where:
  H = halfway vector between view and light direction
  shininess = f(roughness)  // 2.0 to 128.0 based on roughness
```

**Visual Effect**:
- Bright spot where reflection occurs
- Sharp for low roughness (shiny)
- Diffuse for high roughness (matte)

### Ambient
```
ambient = ambientColor × baseColor
```

**Visual Effect**:
- Base illumination for shadowed areas
- Prevents pure black shadows
- Simulates indirect lighting

### Final Color
```
color = ambient + diffuse + specular
```

## Lighting Scenarios Visualized

### Scenario 1: Noon Sun (Overhead)
```
Direction: (0, -1, 0)  ↓
Color: (1.0, 1.0, 0.95) - bright white
Intensity: 2.0
Ambient: (0.3, 0.3, 0.35) - high

Visual: Strong top-down illumination, short shadows
```

### Scenario 2: Sunset (Side Angle)
```
Direction: (0.7, -0.3, 0)  ↘
Color: (1.0, 0.6, 0.3) - warm orange
Intensity: 1.0
Ambient: (0.2, 0.15, 0.2) - medium

Visual: Dramatic side lighting, long shadows, warm tones
```

### Scenario 3: Moonlight (Night)
```
Direction: (0.3, -0.8, 0.5)  ↙
Color: (0.7, 0.8, 1.0) - cool blue
Intensity: 0.5
Ambient: (0.05, 0.06, 0.08) - very low

Visual: Subtle lighting, mostly shadows, cool tones
```

## Light Rotation Demo

```
Position 1 (0°):     Position 2 (90°):    Position 3 (180°):   Position 4 (270°):
   ↓ Light              Light ←              ↑ Light              Light →
   │                        │                    │                    │
   •────────O           •────────O           •────────O           •────────O
Object                Object                Object                Object

Direction:           Direction:            Direction:            Direction:
(1.0, -0.5, 0.0)    (0.0, -0.5, 1.0)     (-1.0, -0.5, 0.0)    (0.0, -0.5, -1.0)
```

## Integration Points

### For Editor Integration
```cpp
// World exposes these methods for UI sliders/controls:
world.set_light_direction(x, y, z);      // Direction vector
world.set_light_color(r, g, b, i);       // Color + intensity
world.set_ambient_light(r, g, b);        // Ambient color

// Example UI pseudocode:
slider_azimuth.onChange(angle => {
    float x = cos(angle);
    float z = sin(angle);
    world.set_light_direction(x, -0.5, z);
});
```

### For Render Pipeline
```cpp
// LitMeshPass automatically:
// 1. Queries World for lighting
// 2. Creates DirectionalLight struct
// 3. Sets on LitMaterial
// 4. Renders all meshes with lighting

render_graph.add_pass(std::make_unique<LitMeshPass>(assets, materials));
```

### For Custom Materials
```cpp
// Future materials can use DirectionalLight:
class MyCustomMaterial : public Material {
    void set_directional_light(const DirectionalLight& light);
    // ... implement custom lighting model
};
```

## Performance Characteristics

- **Material binding**: Once per frame (shared shader)
- **Uniform updates**: Per object (MVP, model, color)
- **Draw calls**: One per visible mesh
- **Shader complexity**: ~50 ALU ops per fragment
- **Memory footprint**: ~100 bytes per light (stack allocation)

## Future Enhancements

1. **Multiple Lights**: Array of DirectionalLight structs
2. **Point Lights**: Position + falloff instead of direction
3. **Spot Lights**: Direction + cone angle + falloff
4. **Shadows**: Shadow mapping pass before lighting
5. **IBL**: Environment maps for ambient
6. **Normal Mapping**: Detail from textures
7. **Light Entities**: Camera-like entities for lights

---

**Created**: 2026-01-31
**Task**: E5 - Lighting v1
**Status**: ✅ Complete
