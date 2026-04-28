# 14 — Roadmap

> Previous: [13 — Scene graph and components](./engine-13-scene-graph.md) · Back to [index](./README.md)

Originally numbered 09. Many of the items below have since been implemented — those are marked ✅. The rest is still the planned trajectory.

| Status | Item |
|---|---|
| ✅ | Camera + matrices (`CameraComponent`, MVP) |
| ✅ | Depth testing + backface culling |
| ✅ | Textures (`Texture` class, stb_image) |
| ✅ | A cube (textured, rotating, with UVs) |
| ✅ | Mesh loading from glTF (cgltf) |
| ✅ | Shaders from disk |
| ✅ | A scene graph (`Scene` + `GameObject` + components) |
| ✅ | JSON-driven materials (`Material::load("…")`) |
| 🟨 | Resize handling + frame-rate independence (partial) |
| 🟨 | More uniform types in `Material` (vec3/vec4/mat4) |
| ⬜ | Lighting (directional + ambient + Phong) |
| ⬜ | Asset manager (dedupe by path) |
| ⬜ | Multi-primitive glTF meshes |
| ⬜ | Hot-reload of shaders/textures |
| ⬜ | Lua scripting (sol2 + Lua) |
| ⬜ | Skeletal animation |
| ⬜ | Shadow mapping |

The list below is the original "everything we could do next." Use it as a menu — pick what teaches you the next thing.

---

## Immediate wins — small, high value

### Resize handling

Right now the viewport is a fixed 1280×720. If you drag the window corner, GL still draws to 1280×720 and the image either stretches or clips.

- **Add**: `glfwSetFramebufferSizeCallback(window, resize_callback)` in `Engine::init`. The callback calls `glViewport(0, 0, new_width, new_height)`.
- **Why now**: one-liner, makes the window feel like a real application.
- **Gotcha**: the callback fires on any OS-driven resize, including DPI changes.

### Frame-rate independence

`Game::update` ignores the `deltatime` parameter, so movement speed scales with framerate. Multiply movement by `deltatime`:

```cpp
constexpr float MOVE_SPEED = 0.5F;       // units per second
if (input.is_key_pressed(GLFW_KEY_W)) offset_y += MOVE_SPEED * deltatime;
```

The engine already computes and passes `deltatime` — just use it.

### More uniform types in `Material`

Today `Material` only stores `float` params. No vec2/vec3/vec4, no mat4. This is the single biggest thing blocking "add a camera" below.

- **Add**: `std::unordered_map<std::string, glm::vec3> vec3_params_;` (and `vec4`, `mat4`), matching `set_param` overloads, and matching `set_uniform` overloads on `ShaderProgram` that call `glUniform3fv`, `glUniform4fv`, `glUniformMatrix4fv`.
- **Better**: a `std::variant<float, glm::vec2, glm::vec3, glm::vec4, glm::mat4>`-based param map so there's one container instead of five.
- **Why**: without this, every new shader input becomes a one-off `ShaderProgram::set_uniform` call scattered through game code.

### Shaders from disk

Move the two shader strings out of `Game::init` into `assets/shaders/basic.vert` and `.frag`. Add a utility that reads a file into a string. Add live reload later by stat-ing the file's mtime each frame and re-creating the program when it changes.

---

## Making it feel 3D

### A camera and transformation matrices

This is the biggest jump. Today the vertex shader does `gl_Position = vec4(a_position + offset, 1.0)`. Real 3D uses three matrices:

- **Model** — where this object is in the world.
- **View** — where the camera is.
- **Projection** — perspective vs orthographic.

The vertex shader becomes `gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);`.

- **Add**: `glm::mat4` support in `Material` (see above), a `Camera` class with position/target/fov/aspect, code that computes `projection = glm::perspective(...)` and `view = glm::lookAt(...)` each frame and uploads them.
- **Prerequisite**: `Material` supporting `mat4`.
- **Read**: `glm` has `lookAt`, `perspective`, `translate`, `rotate`, `scale` helpers — all of what you need.

### Depth testing

Without `glEnable(GL_DEPTH_TEST)`, triangles draw in submission order even if they're "behind" each other. A cube with front-facing triangles drawn last will look right by accident; draw them in a different order and the back pokes through.

- **Add**: `glEnable(GL_DEPTH_TEST)` in `Engine::init` (after `glClearColor`). The depth buffer is already cleared each frame — `glClear(GL_DEPTH_BUFFER_BIT)` is already called.
- **Gotcha**: your default framebuffer needs a depth attachment. GLFW gives you 24 bits of depth by default, so this usually just works.

### Backface culling

For closed meshes (cubes, spheres), triangles facing away from the camera are invisible. Drawing them is wasted work.

- **Add**: `glEnable(GL_CULL_FACE); glCullFace(GL_BACK);` in `Engine::init`.
- **Gotcha**: this assumes your triangles are consistently wound counter-clockwise (the default). If a mesh looks inside-out, either flip the winding or swap `GL_BACK` for `GL_FRONT`.

### A cube

Once you have depth testing and matrices, a cube is the next demo. 8 vertices, 12 triangles (36 indices), rotating via a `glm::rotate`-built model matrix. Seeing it rotate is when 3D "clicks."

---

## Textures and lighting

### Texture support

A `Texture` class wrapping a `GLuint` texture id, constructed from an image file via `stb_image` (already vendored).

- **Add**: `Texture` class with constructor that takes a file path; loads the image; uploads via `glTexImage2D`; stores the id.
- **Also add**: `Material::set_texture(name, shared_ptr<Texture>)` and a uniform of type `sampler2D` in shaders.
- **Also add**: a new `VertexElement` for UVs (texture coordinates) — a 2-float attribute.
- **Read**: [LearnOpenGL's textures chapter](https://learnopengl.com/Getting-started/Textures) is the canonical intro.

### Basic lighting

Directional light + ambient + Phong diffuse. Needs normals as a new vertex attribute, a light-direction uniform, and some shader arithmetic.

- **Add**: normal attribute in `VertexLayout` and `VertexElement`, a `u_light_dir` uniform, a shader that computes `dot(normal, light_dir)` and multiplies the color by it.
- **Read**: [LearnOpenGL's lighting chapters](https://learnopengl.com/Lighting/Colors) walk through ambient, diffuse, specular step by step.

---

## Content and scene management

### Mesh loading from files

Hardcoding vertex arrays is fine for a quad or a cube. For anything else you want to load an OBJ, FBX, or GLTF file.

- **Option A**: write a simple OBJ parser. OBJ is plain text and straightforward (see the [meshes-and-models.md](./meshes-and-models.md) note). Good for learning.
- **Option B**: integrate Assimp. Handles 40+ formats. Heavier dependency but you never think about it again.

### A scene graph

Right now `Game` holds a fixed set of members. A real game has *n* entities that come and go.

- **Add**: an `Entity` or `Node` concept with `Transform`, `Mesh*`, `Material*`. A `Scene` class that holds a list of entities and iterates them in `update` to submit render commands.
- **Extension**: parent/child transforms — a child's world transform is `parent.transform * child.local_transform`. This is how a character's hand follows its arm.

### Asset manager

Deduplication: if two entities ask for "brick.png," only load it once. A map from filename → `shared_ptr<Texture>` does the job. Same for meshes and shaders.

---

## Renderer upgrades

### Command sorting

`RenderQueue::draw` currently walks commands in submission order. Sorting by (shader id, texture id) minimizes `glUseProgram` / `glBindTexture` churn. For transparent objects, sort back-to-front by depth.

- **Add**: a `uint64_t sort_key` computed from (layer, shader hash, depth). `std::sort(commands_.begin(), commands_.end(), ...)` before the loop.
- **Read**: [Christer Ericson's "Order your graphics draw calls around!"](https://realtimecollisiondetection.net/blog/?p=86) is the classic reference.

### Instanced drawing

If you're drawing 1000 identical meshes (trees, rocks), you can upload a per-instance transform buffer and issue *one* `glDrawElementsInstanced` call.

- **Add**: a per-instance attribute (like a `mat4` transform at `location = 2..5`) with `glVertexAttribDivisor(location, 1)` so it advances per-instance rather than per-vertex. An "instanced" command type in the queue.

### Render passes

Distinguish shadow pass, opaque pass, transparent pass, UI pass. Each passes traverses the same scene but filters commands by layer and sorts differently.

### Framebuffer objects

Rendering to a texture (instead of the default framebuffer) opens the door to post-processing, shadow maps, picking, and screen-space effects.

- **Add**: `Framebuffer` class that allocates an FBO, a color texture, and a depth/stencil renderbuffer; `bind()` sets it as the draw target.

---

## Infrastructure

### Logging

`std::cerr` direct calls are fine for a demo. A `Log::info`/`Log::error` layer with levels and timestamps starts paying for itself once you have multiple failure modes.

### A debug HUD

Render frame time and draw call count in the corner. Usually done with [Dear ImGui](https://github.com/ocornut/imgui) — battle-tested and easy to wire up.

### Hot-reload of shaders and assets

Stat file mtimes each frame; if a shader's source changed, re-compile and swap the program. If an image changed, re-upload.

### Scripting

You mentioned wanting to learn Lua/Python/JS bindings. Any of those would let you write gameplay code without recompiling. Lua via `sol2` is the game-industry default; Python via `pybind11` is the nicest binding library to learn; JS via QuickJS is a middle ground. Scripting is a good project once the C++ engine has stable enough abstractions (Entity, Scene, Material) that are worth binding.

### Tests

You have a `tests/` directory. Hook up a test runner (Catch2 is one-header, easy to vendor). Unit tests for input state, vertex layout arithmetic, the render-queue's clearing behavior. Integration tests for "shader compile/link/uniform-set round trip" need a real GL context — harder, but doable with an offscreen context.

---

## Beyond graphics

Once the renderer is comfortable, the next frontiers are:

- **Audio** — SDL_mixer, miniaudio, or OpenAL.
- **Physics** — Bullet, PhysX, or a simpler 2D physics lib if you aren't going full 3D.
- **Networking** — ENet for UDP, or something like nakama if you want backend-as-a-service.
- **Editor** — a tooling layer that inspects and modifies scenes live. Usually built on Dear ImGui.

None of these are graphics engine concerns — but they're what turns "engine" into "game framework."

---

## Final note

An engine is never done — there's always another abstraction to refine, another backend to support, another renderer pass to add. The trap is spending years polishing infrastructure that never runs a game. A good habit: every two or three engine features, pause and ship a small playable demo. It forces you to notice what's still clunky and ignore what doesn't actually matter.

---

## See also

- [08 — Walkthrough](./engine-08-walkthrough.md) — what you already have.
- [graphics-pipeline.md](./graphics-pipeline.md) — the shader pipeline stages you'll be extending.
- [meshes-and-models.md](./meshes-and-models.md) — file formats and loaders for when you move past hardcoded quads.
- [coordinates.md](./coordinates.md) — the coordinate systems you'll start juggling once matrices enter the picture.
