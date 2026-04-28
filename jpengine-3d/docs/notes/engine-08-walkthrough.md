# 08 — The Rectangle Walkthrough

> Previous: [07 — Render queue](./engine-07-render-queue.md) · Next: [10 — Asset paths and file loading](./engine-10-asset-paths.md)

This doc traces the current demo end to end: a colored rectangle that moves with WASD. Every layer of the engine is touched. If you understand this walkthrough, you understand how the engine hangs together.

---

## The demo in one sentence

A rectangle with four vertex colors (red, green, blue, yellow) draws in the center of the window. Holding W/A/S/D moves it up/left/down/right by translating its position in the vertex shader.

---

## Source layout

Three files do the work:

- **`src/main.cpp`** — program entry, sets the app on the engine.
- **`include/game.hpp`** — declares `class Game : public engine::Application` with its state.
- **`src/game.cpp`** — implements `init`, `update`, `destroy`.

The rest is engine code (docs 02–07).

---

## `game.hpp`

```cpp
#pragma once

#include "engine/engine.hpp"
#include "engine/src/graphics/shader-program.hpp"
#include "engine/src/render/material.hpp"
#include "engine/src/render/mesh.hpp"
#include "engine/src/render/render-queue.hpp"

#include <memory>

class Game : public engine::Application {
public:
    bool init() override;
    void update(float deltatime = 0.0F) override;
    void destroy() override;

private:
    std::shared_ptr<engine::ShaderProgram> pshader_program_;
    engine::Material                        material_;
    std::unique_ptr<engine::Mesh>           mesh_;
    float offset_x = 0.0F;
    float offset_y = 0.0F;
};
```

Three resources plus two movement offsets:

- `pshader_program_` — shared ownership because `Material` holds a `shared_ptr` too.
- `material_` — by value; constructed empty, wired up in `init`.
- `mesh_` — `unique_ptr` because `Mesh` is non-copyable and we want lazy construction.
- `offset_x`, `offset_y` — initialized to `0.0F` at the declaration. Reading uninitialized floats is UB; always provide a default.

---

## `Game::init()`

### 1. The shaders

```glsl
// vertex shader
#version 330 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;
out vec3 v_color;

uniform float offset_x;
uniform float offset_y;

void main() {
    gl_Position = vec4(a_position.x + offset_x,
                       a_position.y + offset_y,
                       a_position.z,
                       1.0);
    v_color = a_color;
}
```

```glsl
// fragment shader
#version 330 core
in vec3 v_color;
out vec4 frag_color;

void main() {
    frag_color = vec4(v_color, 1.0);
}
```

What's happening:

- **`layout(location = 0) in vec3 a_position;`** — the vertex shader expects its first input (attribute 0) to be a vec3 position. `a_position` is just a variable name; what matters is the `location = 0`.
- **`layout(location = 1) in vec3 a_color;`** — attribute 1 is a vec3 color.
- **`out vec3 v_color;` / `in vec3 v_color;`** — the vertex stage passes this value to the fragment stage. OpenGL interpolates per-pixel: a pixel halfway between a red and a green vertex gets yellowish.
- **`uniform float offset_x;` / `offset_y`** — per-draw values that `Material` uploads. Same value for every vertex in the draw call.
- **`gl_Position`** — the required output of the vertex shader. A `vec4` in clip space. We emit `(a_position.x + offset_x, ..., ..., 1.0)`.
- **`frag_color`** — the single output of the fragment shader, written to the color buffer.

The mapping:

```
 VBO bytes                      vertex attributes            uniforms
┌────────────────────┐         ┌────────────────┐         ┌──────────────┐
│ x y z  r g b       │ ──────▶ │ a_position(0)  │         │ offset_x     │
│ x y z  r g b       │         │ a_color(1)     │         │ offset_y     │
│ ...                │         └───────┬────────┘         └──────┬───────┘
└────────────────────┘                 ▼                         ▼
                                   vertex shader main() ──▶ gl_Position
                                                             v_color
                                                               │
                                                         (interpolated)
                                                               │
                                                               ▼
                                                         fragment shader main() ──▶ frag_color
```

### 2. Creating the shader program

```cpp
pshader_program_ = engine::Engine::get_instance()
    .get_graphics_api()
    .create_shader_program(vertex_source, fragment_source);

material_.set_shader_program(pshader_program_);
```

See [doc 04](./engine-04-graphics-api.md) for the full flow. In short: compile vs, compile fs, link, return a wrapped handle. If anything failed, `pshader_program_` is nullptr — `init` will report failure at the end.

Wiring the shader into the material means future `material_.bind()` calls will `glUseProgram` this program and upload any stored parameters.

### 3. The vertex data

```cpp
const std::vector<float> vertices = {
    // position          // color
    -0.5F, -0.5F, 0.0F,  1.0F, 0.0F, 0.0F,   // bottom-left  red
     0.5F, -0.5F, 0.0F,  0.0F, 1.0F, 0.0F,   // bottom-right green
     0.5F,  0.5F, 0.0F,  0.0F, 0.0F, 1.0F,   // top-right    blue
    -0.5F,  0.5F, 0.0F,  1.0F, 1.0F, 0.0F,   // top-left     yellow
};
```

Four vertices, 6 floats each → 24 floats total → 96 bytes.

The coordinates are in **normalized device coordinates (NDC)**: x and y go from -1 (left/bottom) to +1 (right/top). z is depth. No projection matrix is applied yet, so NDC is what the shader emits directly. `0.5` on either axis is half the way to the edge.

Colors are in linear RGB, 0–1. Red = `(1,0,0)`, yellow = `(1,1,0)`.

### 4. The index buffer

```cpp
const std::vector<uint32_t> indices = {
    0, 1, 2,
    2, 3, 0,
};
```

Two triangles that together make a quad:

```
  3 ── 2              3─────2
  │    │              │   ╱ │
  │    │    →         │  ╱  │   (triangles 0-1-2 and 2-3-0)
  │    │              │ ╱   │
  0 ── 1              0─────1
```

Why indices? Without them, a quad needs 6 vertices (two triangles × 3 each), duplicating the shared corners. Indexed drawing lets you store 4 unique vertices and reference them twice each.

### 5. The vertex layout

```cpp
engine::VertexLayout layout{
    .elements_ = {
        {.index_ = 0, .size_ = 3, .type_ = GL_FLOAT,
         .offset_ = 0},
        {.index_ = 1, .size_ = 3, .type_ = GL_FLOAT,
         .offset_ = static_cast<uint32_t>(3 * sizeof(float))},
    },
    .stride_ = static_cast<uint32_t>(6 * sizeof(float)),
};
```

Reading this: "Every vertex is 24 bytes (`stride = 6 × 4`). The first 12 bytes are a 3-float position at attribute 0 (offset 0). The next 12 bytes are a 3-float color at attribute 1 (offset 12)."

The `location` numbers here must match the `layout(location = N)` declarations in the vertex shader. If your shader says `location = 1` for color but your layout says `index_ = 2`, GL won't complain — the color attribute just never gets fed, and the fragments come out black or random.

See [doc 06](./engine-06-meshes.md) for what `VertexLayout` does internally.

### 6. Creating the mesh

```cpp
mesh_ = std::make_unique<engine::Mesh>(layout, vertices, indices);
```

This is where the GL calls actually happen — `glGenBuffers`, `glBufferData`, `glGenVertexArrays`, `glVertexAttribPointer`, `glEnableVertexAttribArray`, and the careful unbind order. See [doc 06](./engine-06-meshes.md).

After this line, the GPU has:
- a VBO containing the 24 floats,
- an EBO containing the 6 indices,
- a VAO that knows "position is attribute 0, color is attribute 1, stride 24, EBO bound."

### 7. Returning

```cpp
return pshader_program_ != nullptr;
```

If the shader failed to compile, this returns false — `Engine::init` sees the failure and bails out before calling `run()`. Watch `stderr` for the compile/link error.

---

## `Game::update(float)`

```cpp
void Game::update(float) {
    auto& input = engine::Engine::get_instance().get_input_manager();

    if (input.is_key_pressed(GLFW_KEY_W)) offset_y += 0.01F;
    if (input.is_key_pressed(GLFW_KEY_A)) offset_x -= 0.01F;
    if (input.is_key_pressed(GLFW_KEY_S)) offset_y -= 0.01F;
    if (input.is_key_pressed(GLFW_KEY_D)) offset_x += 0.01F;

    material_.set_param("offset_x", offset_x);
    material_.set_param("offset_y", offset_y);

    engine::RenderCommand command;
    command.material_ = &material_;
    command.mesh_     = mesh_.get();
    engine::Engine::get_instance().get_render_queue().submit(command);
}
```

Three phases:

**Input → state.** Poll each WASD key; bump the offsets. The `0.01F` is movement-per-frame in NDC units. Since the rectangle is 1.0 units wide (-0.5 to +0.5), it takes ~100 frames to move a full rectangle width.

**State → material.** Push the two floats into the material's parameter bag. When the material is bound later, these become `glUniform1f` calls.

**Submit the draw.** Build a `RenderCommand` pointing at our material and mesh. Submit it to the queue. Nothing draws yet.

Note: the `deltatime` parameter is ignored. That means movement speed scales with framerate — at 60 FPS you move 0.6 units per second; at 120 FPS you move 1.2 units per second. The fix is `offset_y += 0.5F * deltatime;`, but the demo doesn't bother yet.

---

## What happens after `update` returns

Back in `Engine::run`:

```cpp
papplication_->update(dt);          // Game::update runs (submits command)

render_queue_.draw(graphics_api_);  // engine drains the queue
```

`RenderQueue::draw` walks its one command:

```cpp
graphics_api.bind_material(command.material_);
graphics_api.bind_mesh(command.mesh_);
graphics_api.draw_mesh(command.mesh_);
```

Which is:

```cpp
// bind_material(material) →
material_.bind() →
    pshader_program_->bind();              // glUseProgram
    for each (name, value) in params:
        pshader_program_->set_uniform();   // glUniform1f

// bind_mesh(mesh) →
mesh_->bind();                             // glBindVertexArray(vao)

// draw_mesh(mesh) →
mesh_->draw();                             // glDrawElements(GL_TRIANGLES, 6, ...)
```

Then `glfwSwapBuffers` flips the framebuffer and the rectangle appears on screen.

---

## Full frame trace

Here's the full sequence of GL calls for a typical frame:

```
glfwPollEvents                            // callback fires, updates InputManager
// compute dt
glClear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT)

// inside Game::update:
  (no GL calls — just state and queue submission)

// inside RenderQueue::draw:
  glUseProgram(program_id)
  glUniform1f(offset_x_location, 0.12F)
  glUniform1f(offset_y_location, 0.05F)
  glBindVertexArray(vao_id)
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr)

glfwSwapBuffers                            // show the frame
```

~6–7 GL calls per frame for this demo. One colored rectangle, moving at user input.

---

## If nothing is on screen

When you hit a black window in a setup like this, common causes are:

1. **Shader compile/link failed** → `create_shader_program` returned null, `init` returned false, `run` never started. Check stderr.
2. **`glUseProgram` never called** (`ShaderProgram::bind` was empty in an earlier version of this engine). Without a program, `glDrawElements` silently draws nothing.
3. **Missing `glClear`** → you see last-frame junk instead of fresh pixels.
4. **Vertex positions outside NDC** → geometry is "there" but off-screen.
5. **Attribute location mismatch** → shader expects position at `location = 0`, layout puts it at `index_ = 1`. GL happily feeds garbage.
6. **Forgot `glEnableVertexAttribArray`** → the VBO data is bound but ignored; the attribute defaults to `(0,0,0,1)` for every vertex.
7. **Uniform name mismatch** → `set_param("offest_x", ...)` (typo) silently does nothing, and the rectangle stays at the origin.

Keep that list handy. Every one of these has been hit at least once during development.

---

## See also

- [02 — Main loop](./engine-02-main-loop.md) — where `update` and `draw` get called.
- [06 — Meshes](./engine-06-meshes.md) — the `Mesh` constructor in detail.
- [07 — Render queue](./engine-07-render-queue.md) — how submission becomes draw calls.
- [coordinates.md](./coordinates.md) — why vertices at `±0.5` end up centered.
