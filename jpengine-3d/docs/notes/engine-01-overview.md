# 01 — Overview

A **game engine** is the reusable infrastructure you build *once* so that each new game, simulation, or tool doesn't start from `main()` staring at a blank screen. A **game** is the specific thing built on top.

The split matters because engines and games change on different timelines:

- **Engine code** evolves slowly — windowing, input, rendering primitives.
- **Game code** evolves fast — levels, entities, gameplay logic.

If you mix them, every gameplay change drags engine internals with it, and you can't reuse anything in the next project. Keeping them separate is the whole point of all the interfaces you'll see in the next few docs (`Application`, `GraphicsApi`, `Material`, etc.) — each one is a seam between "game-side" and "engine-side" code.

---

## What jpengine-3d is today

A small C++23 learning engine built on:

- **GLFW** for window + input platform layer.
- **GLEW** for loading modern OpenGL function pointers.
- **OpenGL 3.3 core** as the rendering backend.
- **glm** for math (vectors, matrices) — vendored but not used yet.
- **stb_image** for image decoding — vendored but not used yet.

It currently supports:

- Opening a window, an OpenGL context, and a main loop.
- Polling keyboard input.
- Compiling/linking GLSL shader programs.
- Uploading vertex and index buffers to the GPU.
- Describing vertex layouts in plain data.
- Bundling shader + parameters as a `Material`.
- Bundling a VAO/VBO/EBO as a `Mesh`.
- Submitting `RenderCommand`s to a queue and drawing them each frame.

It does **not** yet support: cameras, transforms, depth testing, textures, lighting, asset loading, scene graphs, or anything beyond a single `float`-valued material parameter. The [roadmap](./engine-09-roadmap.md) covers what comes next.

---

## Directory layout

```
jpengine-3d/
├── include/
│   ├── engine/
│   │   ├── engine.hpp                  # umbrella header — includes the public API
│   │   └── src/
│   │       ├── application.hpp
│   │       ├── engine.hpp
│   │       ├── graphics/
│   │       │   ├── graphics-api.hpp
│   │       │   ├── shader-program.hpp
│   │       │   └── vertex-layout.hpp
│   │       ├── input/
│   │       │   └── input-manager.hpp
│   │       └── render/
│   │           ├── material.hpp
│   │           ├── mesh.hpp
│   │           └── render-queue.hpp
│   └── game.hpp                        # your game's public header
├── src/
│   ├── engine/
│   │   └── src/                        # mirrors include/engine/src/
│   │       ├── engine.cpp
│   │       ├── application.cpp
│   │       ├── graphics/…
│   │       ├── input/…
│   │       └── render/…
│   ├── game.cpp                        # your game implementation
│   └── main.cpp                        # program entry point
├── vendor/                             # GLFW, GLEW, glm, stb (all vendored)
├── docs/notes/                         # these notes
├── CMakeLists.txt
└── Makefile                            # thin wrapper over cmake+make
```

The `include/` vs `src/` split mirrors the usual C++ layout: public interfaces in `include/`, implementations in `src/`. The two trees mirror each other so you always know where the `.cpp` for a given `.hpp` lives.

---

## The mental model

```
                 ┌──────────────────────────┐
                 │          main()          │
                 └───────────┬──────────────┘
                             │ sets Application
                             ▼
                 ┌──────────────────────────┐
                 │         Engine           │  ◀─── owns the loop,
                 │  (singleton)             │      owns all subsystems
                 └─┬────┬────┬────┬──────┬──┘
                   │    │    │    │      │
           ┌───────┘    │    │    │      └──────────┐
           ▼            ▼    ▼    ▼                 ▼
       ┌───────┐  ┌─────────┐ ┌──────────┐   ┌────────────┐
       │GLFW   │  │ Input   │ │Graphics  │   │RenderQueue │
       │window │  │ Manager │ │  Api     │   │            │
       └───────┘  └─────────┘ └──────────┘   └────────────┘
                                  ▲ uses                ▲ uses
                                  │                     │
                 ┌────────────────┴─────────────────────┘
                 │
                 ▼
           ┌─────────────┐
           │ Application │   ◀─── YOU implement this
           │ (your Game) │      (Game : public Application)
           └─────────────┘
```

The rule of thumb: **the Engine calls into your Application; your Application never owns the loop.** This "inversion of control" is what makes the same engine reusable across different games.

---

## What to read next

1. **[02 — Main loop](./engine-02-main-loop.md)** — how `Application` and the run loop work.
2. **[03 — Input](./engine-03-input.md)** — translating GLFW callbacks into polled state.
3. **[04 — Graphics API](./engine-04-graphics-api.md)** — why we wrap OpenGL.
4. **[05 — Shaders & Materials](./engine-05-shaders-materials.md)** — the shader pipeline.
5. **[06 — Meshes](./engine-06-meshes.md)** — vertex/index buffers + layouts.
6. **[07 — Render queue](./engine-07-render-queue.md)** — deferred draw submission.
7. **[08 — Walkthrough](./engine-08-walkthrough.md)** — the rectangle + WASD demo tied together.
8. **[09 — Roadmap](./engine-09-roadmap.md)** — everything this engine doesn't do yet.

If you haven't read the OpenGL concept notes, start with [opengl-stack.md](./opengl-stack.md) — this guide assumes you roughly know what OpenGL, GLFW, and GLEW each are.
