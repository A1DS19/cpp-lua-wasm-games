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
| 05 | [engine-05-shaders-materials.md](./engine-05-shaders-materials.md) | `ShaderProgram` + `Material`. Uniform caching. Shader + params bundling. |
| 06 | [engine-06-meshes.md](./engine-06-meshes.md) | `VertexLayout`, `VertexElement`, `Mesh`. VAO/VBO/EBO tied together. |
| 07 | [engine-07-render-queue.md](./engine-07-render-queue.md) | `RenderCommand` + `RenderQueue`. Deferred submission. |
| 08 | [engine-08-walkthrough.md](./engine-08-walkthrough.md) | The rectangle + WASD demo, annotated end-to-end. |
| 09 | [engine-09-roadmap.md](./engine-09-roadmap.md) | Gaps: camera, depth, textures, scene graph, scripting, etc. |

## OpenGL concept notes

Background reading on the graphics stack. Read when you hit a concept you don't recognize in the engine guide.

| File | Topic |
|------|-------|
| [opengl-stack.md](./opengl-stack.md) | What OpenGL, GLFW, and GLEW each do — and why GLEW exists at all. |
| [coordinates.md](./coordinates.md) | 3D axes (`x` = width, `y` = height, `z` = depth), right-handed vs left-handed, gotchas. |
| [meshes-and-models.md](./meshes-and-models.md) | How models become triangles, vertex/index buffers, file formats, loading with Assimp. |
| [graphics-pipeline.md](./graphics-pipeline.md) | Stages of the OpenGL pipeline, what vertex and fragment shaders actually do, fragment vs pixel. |
| [vao-vbo-shaders.md](./vao-vbo-shaders.md) | What VAOs, VBOs, EBOs, and shader programs are; correct init order; minimal draw loop. |
