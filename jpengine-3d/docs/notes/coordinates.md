# 3D Coordinates — Axes & Handedness

## The axes

A point in 3D is `(x, y, z)`:

| Axis | Meaning          |
|------|------------------|
| `x`  | **width**  — left ↔ right |
| `y`  | **height** — down ↔ up    |
| `z`  | **depth**  — far ↔ near   |

Same as a 2D Cartesian plane with one axis added for depth.

### Example

```
(0.0, 0.0,  0.0)   // at the origin
(1.0, 0.0,  0.0)   // 1 unit right
(0.0, 0.5,  0.0)   // half unit up
(0.0, 0.0, -5.0)   // 5 units into the screen (OpenGL RH)
```

## Right-handed vs left-handed

Two conventions for which way `+Z` points:

### Right-handed (RH) — used by OpenGL, Vulkan, Blender, glTF
- **+X** → right
- **+Y** → up
- **+Z** → **toward the viewer** (out of the screen)
- Forward (into the scene) is **−Z**
- Hand rule: right hand, thumb=X, index=Y, middle=Z

### Left-handed (LH) — used by DirectX, Unity, Unreal
- **+X** → right
- **+Y** → up
- **+Z** → **away from the viewer** (into the screen)
- Forward is **+Z**

## What actually changes in code

| Thing                       | RH (OpenGL)     | LH (DirectX)    |
|-----------------------------|-----------------|-----------------|
| Camera forward              | `(0, 0, -1)`    | `(0, 0, +1)`    |
| Cross product direction     | right-hand rule | left-hand rule  |
| Projection Z range          | `[-near,-far] → [-1,+1]` | `[near,far] → [0,1]` |
| Front-face winding          | CCW (default)   | CW (default)    |
| Rotation sign around +axis  | CCW positive    | CW positive     |

## Gotchas when mixing

1. **Importing models**: a `.fbx` from Maya (RH) into Unity (LH) flips axes — that's why characters sometimes face backwards.
2. **Cross product for normals**: `cross(edge1, edge2)` points opposite between hands — face culling inverts.
3. **Quaternions / euler angles**: rotation signs flip.
4. **Matrix helpers**: GLM has both (`glm::perspective` = RH, `glm::perspectiveLH` = LH). Pick one and stay consistent.

## For jpengine-3d

Go **right-handed**. OpenGL + GLM defaults to it, most asset formats assume it, and the whole tutorial ecosystem uses it.

```cpp
glm::mat4 view = glm::lookAt(
    glm::vec3(0, 0, 3),   // eye
    glm::vec3(0, 0, 0),   // target
    glm::vec3(0, 1, 0));  // up

glm::mat4 proj = glm::perspective(
    glm::radians(60.0f), aspect, 0.1f, 100.0f);
```

### Muscle-memory trick

Hold your right hand in front of the monitor:
- Thumb right (+X)
- Index up (+Y)
- Middle finger points at your face (+Z)
