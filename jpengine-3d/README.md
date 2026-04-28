# jpengine-3d

A small 3D game engine written in modern C++23, built as a learning project to render a textured world with a perspective camera. Native-only (no WASM) for now.

## What it does

- **OpenGL 3.3 core** rendering via GLFW + GLEW
- **Scene graph** with `GameObject`s, components, and a render queue
- **JSON-driven materials** (shader paths, uniforms, textures) loaded via nlohmann/json
- **Texture / mesh / shader-program** wrappers around raw GL resources
- **Asset path resolution** via a compile-time-baked `JPENGINE_ASSETS_DIR` macro
- **Header-only vendored dependencies** for reproducible builds
- **AddressSanitizer + UBSan** in debug builds; `-Werror` on warnings

## Tech stack

| Library | Purpose |
|---------|---------|
| [GLFW 3.4](https://www.glfw.org/) | Window, input, OpenGL context |
| [GLEW](https://glew.sourceforge.net/) | OpenGL function loader |
| [GLM](https://github.com/g-truc/glm) | Math (vectors, matrices, quaternions) |
| [stb_image](https://github.com/nothings/stb) | PNG/JPG decoding |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing for material files |

All dependencies are vendored under `vendor/` — no system-package install required.

## Requirements

- CMake 3.25+
- C++23 compiler:
  - Clang 15+ (macOS/Linux)
  - GCC 13+ (Linux)
  - MSVC 2022 (Windows, untested)
- OpenGL 3.3+ capable GPU/driver

```bash
# macOS
brew install cmake

# Linux (Ubuntu)
sudo apt install cmake build-essential libgl1-mesa-dev xorg-dev
```

## Build

```bash
cd jpengine-3d
make build       # debug build (AddressSanitizer + UBSan + -Werror)
make run         # build and run
make release     # optimized release build
```

Output lands in `bin/main`. Working directory doesn't matter — assets resolve through `JPENGINE_ASSETS_DIR`, baked at configure time.

See `make help` for the full target list (formatting, doxygen, valgrind, etc.).

## Project structure

```
jpengine-3d/
├── src/
│   ├── engine/src/           # Engine implementation (.cpp)
│   ├── utils/                # File loaders, stb impl
│   ├── game.cpp              # Application implementation
│   ├── main.cpp              # Entry point
│   └── test-obj.cpp          # Demo cube object
├── include/
│   ├── engine/
│   │   ├── engine.hpp        # Umbrella public header
│   │   └── src/              # Engine internal headers
│   │       ├── application.hpp
│   │       ├── engine.hpp
│   │       ├── graphics/     # GraphicsApi, ShaderProgram, Texture, VertexLayout
│   │       ├── input/        # InputManager
│   │       ├── render/       # Material, Mesh, RenderQueue
│   │       └── scene/        # Scene, GameObject, Component, components/
│   └── utils/                # asset-path, file-utils, macros
├── assets/
│   ├── shaders/              # GLSL files
│   ├── textures/             # PNGs
│   └── materials/            # *.mat (JSON)
├── vendor/                   # Bundled third-party libraries
│   ├── glfw-3.4/             # Built via add_subdirectory
│   ├── glew/                 # Built as a static library
│   ├── glm/                  # Header-only
│   ├── stb/                  # Header-only (impl in src/utils/stb-impl.cpp)
│   └── json/                 # Header-only (nlohmann/json)
├── docs/
│   └── notes/                # Conceptual notes — read in order
├── bin/                      # Build output
├── CMakeLists.txt
└── Doxyfile
```

## Engine concepts

### Lifecycle

`Engine` is a singleton. Each frame:

```
glfwPollEvents → Application::update(dt) → RenderQueue draw → swap buffers
```

`Application` is the user-facing extension point — subclass it, override `init`, `update`, `destroy`. See `src/game.cpp`.

### Scene graph

A `Scene` owns `GameObject`s. Each `GameObject` carries:
- transform (`position`, `rotation` as `glm::quat`, `scale`)
- a list of `Component`s (typed by a runtime ID injected via the `COMPONENT(...)` macro)
- optional children

Built-in components: `CameraComponent`, `MeshComponent`, `PlayerControllerComponent`.

### Materials

Materials are described in JSON (`assets/materials/*.mat`):

```json
{
    "shader": {
        "vertex":   "shaders/vertex.glsl",
        "fragment": "shaders/fragment.glsl"
    },
    "params": {
        "textures": [
            { "name": "brick_texture", "path": "textures/brick.png" }
        ]
    }
}
```

`Material::load("materials/brick.mat")` reads the file, compiles the shader pair, loads the textures, and returns a ready-to-bind `std::shared_ptr<Material>`. All paths are relative to `assets/`.

### Asset paths

```cpp
#include "utils/asset-path.hpp"
#include "utils/file-utils.hpp"

auto p   = utils::asset_path("textures/wall.png");          // std::filesystem::path
auto src = utils::read_asset_text("shaders/basic.vert");    // std::string, throws on miss
```

`JPENGINE_ASSETS_DIR` is defined in `CMakeLists.txt` at configure time and baked into the binary, so the executable finds its assets regardless of where it's launched from.

### Error handling policy

- **Exceptions** for fatal load/init errors (asset missing, shader compile fail, GL init fail). Caught once at the top of `main`.
- **`assert`** for programmer errors and impossible-state checks. Disappears in release.
- **Return values** for per-frame conditions. No throws in `update()` / `draw()`.

## Documentation

- [`docs/notes/`](./docs/notes/README.md) — conceptual walkthrough of the engine and OpenGL primer. Read the `engine-*.md` files in order.
- Doxygen: `make docs && open docs/html/index.html`

## Status

Early. The current demo renders a textured spinning cube with a perspective camera. Roadmap (see [`docs/notes/engine-14-roadmap.md`](./docs/notes/engine-14-roadmap.md)):

- [ ] Lighting (directional, point)
- [ ] glTF model loading (assimp or cgltf)
- [ ] First-person controller with collision
- [ ] Lua scripting (sol2 + Lua, matching jpengine-2d's pattern)
- [ ] Skeletal animation
- [ ] Shadow mapping

## Sister project

[**jpengine-2d**](../jpengine-2d/README.md) — a 2D engine with Lua scripting and WASM support. Earlier and more feature-complete; jpengine-3d borrows its architectural patterns (asset path macros, header-only vendoring, error-handling policy).
