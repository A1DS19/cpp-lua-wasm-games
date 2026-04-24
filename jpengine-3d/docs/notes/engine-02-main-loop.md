# 02 — The Main Loop

> Previous: [01 — Overview](./engine-01-overview.md) · Next: [03 — Input](./engine-03-input.md)

Every interactive program has a **main loop**. In a game engine the loop roughly looks like:

```
while (!should_close) {
    poll_input();
    compute_delta_time();
    update_game_state(dt);
    render();
    swap_buffers();
}
```

Your game's logic needs to run once per frame, forever. The question is: *who owns the loop?*

---

## Inversion of control

If **your game** owns the loop, every game re-implements it — window management, input, delta time, swap buffers. That's a lot of boilerplate, and the engine ends up being a library that you have to orchestrate yourself.

If **the engine** owns the loop, your game only has to say what to do *inside* a frame. The engine takes care of "how often" and "in what order." You plug in a callback the engine calls each tick.

This is called **inversion of control** (or the "Hollywood principle": don't call us, we'll call you). Most frameworks work this way — the engine runs; your game is a passenger with side effects.

jpengine-3d implements this with a tiny interface called `Application`.

---

## The `Application` interface

`include/engine/src/application.hpp`:

```cpp
namespace engine {

class Application {
public:
    virtual ~Application() = default;

    virtual bool init() = 0;
    virtual void update(float deltatime = 0.0F) = 0;
    virtual void destroy() = 0;

    void set_needs_to_be_closed(bool value) noexcept;
    [[nodiscard]] bool needs_to_be_closed() const noexcept;

private:
    bool needs_to_be_closed_{false};
};

} // namespace engine
```

Three pure virtuals. That's the entire contract a game has to satisfy:

| Method      | When the engine calls it                 | You put…                              |
|-------------|------------------------------------------|---------------------------------------|
| `init()`    | once, after the window + GL are ready    | asset loading, shader compile, setup  |
| `update(dt)`| every frame, after input is polled       | gameplay logic + drawing submission   |
| `destroy()` | once, before tearing down the window     | free resources                        |

`init()` returns `bool` so the engine can bail out cleanly if setup fails (e.g. a shader didn't compile).

`needs_to_be_closed_` is a flag the app can set to tell the engine "please exit." It's default-initialized to `false`. (An earlier bug in this engine was that this field was *not* initialized — the rendering loop read uninitialized memory and sometimes exited instantly on startup. Always initialize every `bool`, `int`, or pointer member in its declaration.)

The game implements this interface:

```cpp
// include/game.hpp
class Game : public engine::Application {
public:
    bool init() override;
    void update(float deltatime = 0.0F) override;
    void destroy() override;

private:
    // ... game state members
};
```

---

## The Engine class

`include/engine/src/engine.hpp`:

```cpp
namespace engine {
class Engine {
public:
    static Engine& get_instance();         // singleton

    bool init();
    void run();
    void destroy();

    void set_application(Application* papplication) noexcept;

    // accessors for subsystems
    GLFWwindow*   get_window();
    InputManager& get_input_manager();
    GraphicsApi&  get_graphics_api();
    RenderQueue&  get_render_queue();

private:
    Engine() = default;
    // deleted copy/move

    std::unique_ptr<Application>           papplication_;
    std::chrono::steady_clock::time_point  last_time_point_;
    GLFWwindow*                            pwindow_ = nullptr;
    InputManager                           input_manager_;
    GraphicsApi                            graphics_api_;
    RenderQueue                            render_queue_;
};
}
```

The `Engine` **owns** every subsystem by value — they're constructed with the engine, destructed with it, and live for the program's lifetime. The only thing the engine heap-allocates is your `Application` (because you pass it in as a pointer).

### Why a singleton?

`get_instance()` returns a Meyers singleton (a local `static` inside the accessor). This is the simplest way to give *any* code in the project access to the engine — gameplay code calls `engine::Engine::get_instance().get_input_manager()` without having to thread a pointer through every class.

**Tradeoffs:**
- ✅ Dead simple. Zero wiring.
- ❌ Hidden dependency. A function that reads input doesn't *look* like it depends on the engine.
- ❌ Testing is harder — everything that uses the engine needs a real engine to be alive.

For a one-game project, this is fine. In a larger codebase you'd probably pass an `Engine&` or a narrower interface explicitly.

---

## `main.cpp` — putting it together

`src/main.cpp`:

```cpp
int main() {
    Game* game = new Game;
    auto& eng = engine::Engine::get_instance();

    eng.set_application(game);

    if (eng.init()) {
        eng.run();
    }

    eng.destroy();

    return EXIT_SUCCESS;
}
```

The flow:

1. Construct the game (on the heap; the engine takes ownership via `unique_ptr`).
2. Hand it to the engine via `set_application`.
3. Call `init()` → if that returns `true`, the setup worked.
4. Call `run()` → this **blocks** until the user closes the window.
5. Call `destroy()` → tear everything down.

---

## Engine::init — one-time setup

`src/engine/src/engine.cpp` (sketch):

```cpp
bool Engine::init() {
    if (!papplication_) return false;

    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);   // Linux / X11
    if (!glfwInit()) return false;

    // OpenGL context hints — MUST be before glfwCreateWindow
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    pwindow_ = glfwCreateWindow(1280, 720, "jpengine-3d", nullptr, nullptr);
    if (!pwindow_) { glfwTerminate(); return false; }

    glfwMakeContextCurrent(pwindow_);
    glfwSetKeyCallback(pwindow_, key_callback);      // input wiring — see doc 03

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { /* cleanup */ return false; }

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);            // default clear color

    return papplication_->init();                    // let the game set up
}
```

A few things worth calling out:

- **Context hints before window creation.** GLFW's window hints are sticky — they apply to the *next* `glfwCreateWindow`. If you set them after, they're ignored.
- **GLEW after context.** GLEW loads function pointers by asking the current OpenGL context. No context → nothing to ask.
- **`glewExperimental = GL_TRUE`** — needed for core profile contexts; without it GLEW sometimes misses extension entry points.
- **`glClearColor` once at init.** The clear color is a piece of global GL state. You only need to set it when you want it to change.

The engine returns whatever `papplication_->init()` returns. If your game's init fails (e.g. a shader didn't compile), the whole engine bails out cleanly.

---

## Engine::run — the loop

```cpp
void Engine::run() {
    if (!papplication_ || !pwindow_) return;

    last_time_point_ = std::chrono::steady_clock::now();

    while (!papplication_->needs_to_be_closed()
           && !glfwWindowShouldClose(pwindow_)) {
        glfwPollEvents();

        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last_time_point_).count();
        last_time_point_ = now;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        papplication_->update(dt);

        render_queue_.draw(graphics_api_);

        glfwSwapBuffers(pwindow_);
    }
}
```

Frame lifecycle, top to bottom:

1. **`glfwPollEvents`** — process pending OS events (keyboard, mouse, resize…). This is what ends up triggering the key callback that updates `InputManager`.
2. **Delta time.** How long the previous frame took, in seconds. Gameplay code uses this to move things at a consistent speed regardless of framerate.
3. **`glClear`** — wipe the color (and depth) buffer. Without this, the previous frame's pixels stick around or the buffer contents are undefined (this causes flickering).
4. **`papplication_->update(dt)`** — your game runs. It reads input, updates state, submits draw commands to the `RenderQueue`.
5. **`render_queue_.draw(graphics_api_)`** — the engine drains whatever the game submitted and issues GL draw calls.
6. **`glfwSwapBuffers`** — flip the back buffer to the screen.

### Why `steady_clock`?

Two chrono clocks compete for the "measure real time" job:

- `system_clock` — wall clock. Can jump backwards (NTP, DST, user changes the time).
- `steady_clock` — monotonic. Only increases. No jumps.

For frame deltas you want steady_clock. Never use `system_clock` for anything game-loop-related — you'll get negative dt values when the clock adjusts and your game will teleport or freeze.

(On Linux GCC, `high_resolution_clock` is an alias for `system_clock`, not `steady_clock`. Don't use it either. The name is misleading.)

### Why clear *before* update?

You could also clear after `update` and before `draw`. The effect is the same. Clearing up front just makes the update's drawing-related side effects (debug overlays, etc.) guaranteed to be on a clean buffer.

---

## Engine::destroy

```cpp
void Engine::destroy() {
    if (papplication_) {
        papplication_->destroy();   // your game cleans up
        papplication_.reset();
    }
    if (pwindow_) {
        glfwDestroyWindow(pwindow_);
        pwindow_ = nullptr;
    }
    glfwTerminate();
}
```

Teardown order is the reverse of init: app first (it may still hold references to GL objects), then window, then GLFW itself.

---

## What's naive about this

- **Fixed timestep is missing.** Gameplay runs at whatever the render framerate is. If your framerate drops, physics slows down. A more robust loop runs `update(dt)` at a fixed dt in a loop (often called "Fix Your Timestep!" — Glenn Fiedler has a classic blog post on this).
- **No resize handling.** If you drag the window corner, the viewport stays at 1280×720 and the image stretches. Fix: register a `glfwSetFramebufferSizeCallback` that calls `glViewport(0, 0, w, h)`.
- **No VSync config.** GLFW enables it by default, but you might want `glfwSwapInterval(0)` to disable for performance testing.
- **`new Game` in main is the one raw `new`** — once the engine takes it via `set_application`, ownership lives in a `unique_ptr`. A cleaner alternative is `eng.set_application(new Game)` inline, or a factory.

---

## See also

- [03 — Input](./engine-03-input.md) — how `glfwPollEvents` + callbacks become per-frame key state.
- [07 — Render queue](./engine-07-render-queue.md) — what `render_queue_.draw(...)` actually does.
- [opengl-stack.md](./opengl-stack.md) — background on what GLFW and GLEW are each doing.
