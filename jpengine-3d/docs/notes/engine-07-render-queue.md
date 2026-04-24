# 07 — The Render Queue

> Previous: [06 — Meshes](./engine-06-meshes.md) · Next: [08 — Walkthrough](./engine-08-walkthrough.md)

## Why this exists

Naive rendering looks like this:

```cpp
void Game::update(float) {
    material1_.bind();
    mesh1_.bind();
    mesh1_.draw();

    material2_.bind();
    mesh2_.bind();
    mesh2_.draw();
}
```

Gameplay code issues GL calls directly. This works for two objects. It falls apart for a hundred, because:

- **You can't sort draws.** GL state changes (binding a shader, binding a texture) are expensive. If you draw A1, B1, A2, B2, you change state 4 times. If you could sort into A1, A2, B1, B2, you'd change state twice.
- **You can't cull against the camera before issuing work.** Once you called `draw()`, the GPU is already processing vertices that might be offscreen.
- **You can't batch.** If three objects share a material, you could draw them as one indirect/instanced call — but only if you collected them first.
- **You can't mix systems.** Gameplay code, UI code, and debug-viz code all want to draw. If each calls GL directly in random order, you have no control.

The **render queue** pattern decouples *what to draw* from *when and how*:

1. **Submission phase** (during `update`): gameplay adds `RenderCommand`s to a queue. No GL calls.
2. **Dispatch phase** (end of frame): the engine sorts/culls/batches the queue, then issues the actual draw calls.

In jpengine-3d the dispatch phase is currently trivial — just walk the queue and issue calls in order — but the structure is in place for smarter logic later.

---

## The types

`include/engine/src/render/render-queue.hpp`:

```cpp
namespace engine {

class Mesh;
class Material;

struct RenderCommand {
    Mesh*     mesh_     = nullptr;
    Material* material_ = nullptr;
};

class RenderQueue {
public:
    void submit(const RenderCommand& command);
    void draw(class GraphicsApi& graphics_api);

private:
    std::vector<RenderCommand> commands_;
};

}
```

**`RenderCommand`** is a plain data bag: "render this mesh with this material." No ownership — just pointers. The command producer (`Game::update`) keeps the `Mesh` and `Material` alive; the queue just references them.

**`RenderQueue`** holds a vector of commands. `submit` pushes; `draw` drains.

---

## Submission

`Game::update`:

```cpp
engine::RenderCommand command;
command.material_ = &material_;
command.mesh_     = mesh_.get();

engine::Engine::get_instance().get_render_queue().submit(command);
```

No GL calls happen here. Nothing is drawn yet. The command is just recorded.

You can submit multiple commands per frame:

```cpp
// pseudo-code for a later demo with many objects
for (auto& enemy : enemies) {
    RenderCommand cmd;
    cmd.material_ = &enemy_material_;
    cmd.mesh_ = enemy.mesh();
    render_queue.submit(cmd);
}
```

---

## Dispatch

`src/engine/src/render/render-queue.cpp`:

```cpp
void RenderQueue::submit(const RenderCommand& command) {
    commands_.push_back(command);
}

void RenderQueue::draw(GraphicsApi& graphics_api) {
    for (auto& command : commands_) {
        graphics_api.bind_material(command.material_);
        graphics_api.bind_mesh(command.mesh_);
        graphics_api.draw_mesh(command.mesh_);
    }

    commands_.clear();
}
```

The draw phase:

1. Iterates every submitted command.
2. For each: bind material (which binds the shader and uploads uniforms — see [doc 05](./engine-05-shaders-materials.md)), bind the mesh (which binds the VAO), issue the draw call.
3. **Clears the queue.** Commands live for one frame; the game has to re-submit next frame.

That "clear every frame" is intentional. Gameplay state changes — which entities exist, where they are — so the queue has to be rebuilt. It also makes the system simple to reason about: each frame starts with an empty queue.

---

## Where the queue is drained

`Engine::run`:

```cpp
while (running) {
    glfwPollEvents();
    // ... compute dt
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    papplication_->update(dt);       // game submits commands

    render_queue_.draw(graphics_api_); // engine drains them

    glfwSwapBuffers(pwindow_);
}
```

The order — update before draw — means a command submitted *this frame* gets drawn *this frame*. The game never has to know about the frame boundary explicitly.

---

## Why separate bind_material from draw_mesh?

You might wonder why `graphics_api` has three methods (`bind_material`, `bind_mesh`, `draw_mesh`) instead of one `draw(command)`. The split is so the queue can later:

- Call `bind_material` only when the material *changes* between adjacent commands (state-change caching).
- Call `bind_mesh` + `draw_mesh` multiple times for the same material (batching).
- Skip the whole binding if the material is already bound (first-pass optimization).

With a monolithic `draw(command)` you'd re-bind everything on every command — cheap GL calls, but on a console or in a scene with thousands of objects it adds up.

Currently none of these optimizations are applied. The split exists so they can be added without changing `GraphicsApi`'s caller-facing API.

---

## What's naive about this

- **No sorting.** Commands are drawn in submission order. If you submit transparent objects mixed with opaque ones, they'll render out of order and blend incorrectly. A sensible policy is opaque back-to-front… wait, that's transparent. Opaque front-to-back (early-Z wins), transparent back-to-front (alpha blending needs it). None of this exists yet.
- **No culling.** A mesh 100 units behind the camera still consumes a draw call.
- **No batching.** Two commands with the same material still re-bind it twice.
- **No passes / layers.** Real engines distinguish shadow pass, opaque pass, transparent pass, UI pass. Each pass sorts differently. Here there's one flat queue.
- **No depth state per command.** A transparent object should disable depth writes; a UI element should disable depth tests. Per-command render state isn't modeled.
- **Commands hold raw pointers.** If the `Mesh` is destroyed between submit and draw, `draw_mesh` calls into freed memory. For a single-threaded engine where submit+draw happen in the same `update` cycle, this is fine. Multithreading would need a different ownership model.

---

## A sneak peek at what a richer queue looks like

```cpp
struct RenderCommand {
    Mesh*      mesh;
    Material*  material;
    glm::mat4  transform;
    float      depth_key;     // distance to camera, for sorting
    uint32_t   sort_layer;    // opaque / transparent / UI
};

class RenderQueue {
public:
    void submit(const RenderCommand&);
    void draw(GraphicsApi&);

private:
    void sort();              // stable sort by (layer, material, depth)
    void draw_pass(uint32_t layer, GraphicsApi&);

    std::vector<RenderCommand> commands_;
};
```

You can see the structure staying roughly the same — it's the *policy* inside `draw()` that gets smarter over time.

---

## See also

- [02 — Main loop](./engine-02-main-loop.md) — where the queue is drained.
- [05 — Shaders & Materials](./engine-05-shaders-materials.md) — what `bind_material` does.
- [06 — Meshes](./engine-06-meshes.md) — what `bind_mesh` and `draw_mesh` do.
- [08 — Walkthrough](./engine-08-walkthrough.md) — the submit/draw path end to end.
