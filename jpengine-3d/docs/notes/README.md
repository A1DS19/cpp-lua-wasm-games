# jpengine-3d Notes

Conceptual notes and learning references for the 3D engine.

## Engine guide

A walkthrough of jpengine-3d itself, written as a guide for someone new to building engines. Read in order.

| # | File | Topic |
|---|------|-------|
| 01 | [engine-01-overview.md](./engine-01-overview.md) | What the engine is, engine vs game, directory layout, mental model. |
| 02 | [engine-02-main-loop.md](./engine-02-main-loop.md) | `Application` interface, the run loop, frame lifecycle. Inversion of control. |
| 03 | [engine-03-input.md](./engine-03-input.md) | `InputManager`. GLFW callback → polled state. |
| 04 | [engine-04-graphics-api.md](./engine-04-graphics-api.md) | `GraphicsApi` wrapper. Why not call GL directly from game code. |
| 05 | [engine-05-shaders-materials.md](./engine-05-shaders-materials.md) | `ShaderProgram` + `Material`. Uniform caching. JSON-driven `Material::load`. |
| 06 | [engine-06-meshes.md](./engine-06-meshes.md) | `VertexLayout`, `VertexElement`, `Mesh`. VAO/VBO/EBO tied together. |
| 07 | [engine-07-render-queue.md](./engine-07-render-queue.md) | `RenderCommand` + `RenderQueue`. Deferred submission. |
| 08 | [engine-08-walkthrough.md](./engine-08-walkthrough.md) | The textured cube demo, annotated end-to-end. |
| 10 | [engine-10-asset-paths.md](./engine-10-asset-paths.md) | `JPENGINE_ASSETS_DIR`, `asset_path`, `read_asset_text`. Compile-time path baking. |
| 11 | [engine-11-textures.md](./engine-11-textures.md) | `Texture` class, stb_image, the implementation-TU pattern, UV origin. |
| 12 | [engine-12-mesh-loading.md](./engine-12-mesh-loading.md) | Loading `.gltf` with cgltf — accessor → interleaved VBO; UV flip; gotchas. |
| 13 | [engine-13-scene-graph.md](./engine-13-scene-graph.md) | `Scene` + `GameObject` + `Component`. Transforms, type IDs, the `COMPONENT(...)` macro. |
| 14 | [engine-14-roadmap.md](./engine-14-roadmap.md) | What's done, what's next. (Originally numbered 09.) |

> Note: there's no `engine-09` — it became `engine-14-roadmap.md` when notes 10–13 were inserted.

## OpenGL concept notes

Background reading on the graphics stack. Read when you hit a concept you don't recognize in the engine guide.

| File | Topic |
|------|-------|
| [opengl-stack.md](./opengl-stack.md) | What OpenGL, GLFW, and GLEW each do — and why GLEW exists at all. |
| [coordinates.md](./coordinates.md) | 3D axes (`x` = width, `y` = height, `z` = depth), right-handed vs left-handed, gotchas. |
| [meshes-and-models.md](./meshes-and-models.md) | How models become triangles, vertex/index buffers, file formats, loading with Assimp. |
| [graphics-pipeline.md](./graphics-pipeline.md) | Stages of the OpenGL pipeline, what vertex and fragment shaders actually do, fragment vs pixel. |
| [vao-vbo-shaders.md](./vao-vbo-shaders.md) | What VAOs, VBOs, EBOs, and shader programs are; correct init order; minimal draw loop. |

## Quick reference — conventions used in the engine

| Concept | Choice | Why |
|---------|--------|-----|
| Coordinate system | Right-handed | OpenGL/GLM defaults; matches glTF and Blender. |
| Rotation representation | `glm::quat` | No gimbal lock, cheap to compose, no Euler-order ambiguity. |
| Asset paths | `JPENGINE_ASSETS_DIR` macro baked at configure time | Working-directory-independent; predictable. |
| Material format | JSON (nlohmann/json) | Editable without recompile; minimal parser overhead. |
| 3D model format | glTF 2.0 via cgltf | Modern, single-header loader, Blender-friendly. |
| Image format | PNG via stb_image | Single-header, ubiquitous, alpha support. |
| Texture upload | RGBA8 + mipmaps + trilinear | Sane default; one branch removed in upload code. |
| UV convention | Bottom-left origin (GL native); glTF UVs flipped V at load. | Manual UVs match what authors expect; glTF reconciled in one place. |
| Error handling | Exceptions for fatal load/init; `assert` for impossible states; return values per-frame | No exceptions in hot loops; one catch point in `main`. |
| GL resource ownership | RAII wrapper (`Texture`, `Mesh`, `ShaderProgram`) holds the GL handle | Destructor calls `glDelete*`; move-only, never copyable. |
| Component type IDs | `COMPONENT(T)` macro injects `static type_id()` + virtual `get_type_id()` | Cheap integer compare for component lookup; no RTTI. |
| Single-header impl pattern | One `.cpp` per lib defines `*_IMPLEMENTATION` | Avoid duplicate-symbol linker errors. |
| Vendoring | Header-only deps committed under `vendor/` | Reproducible builds; offline-capable. |
