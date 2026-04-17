# jpengine-2d

A 2D game engine written in C++ with Lua scripting support, targeting both native Linux/macOS and WebAssembly (browser).

[![Linux Build](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-linux.yml/badge.svg)](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-linux.yml)
[![WASM Build](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-wasm.yml/badge.svg)](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-wasm.yml)

## What it does

- Entity-Component-System (ECS) architecture via **EnTT**
- Lua scripting via **sol2** — game logic is written in Lua
- Rendering with **SDL2** + **OpenGL ES 3** (WebGL2 in the browser)
- Math with **GLM**
- Image loading with **stb_image**
- Compiles to WebAssembly with **Emscripten**

## Tech stack

| Library | Purpose |
|---------|---------|
| [EnTT](https://github.com/skypjack/entt) | Entity-Component-System |
| [sol2](https://github.com/ThePhD/sol2) | Lua bindings for C++ |
| [Lua 5.x](https://www.lua.org/) | Scripting language |
| [SDL2](https://www.libsdl.org/) | Window, input, audio |
| [GLM](https://github.com/g-truc/glm) | Math (vectors, matrices) |
| [stb](https://github.com/nothings/stb) | Image loading |
| [Emscripten](https://emscripten.org/) | C++ → WebAssembly compiler |

## Requirements

### Native (Linux / macOS)

```bash
# Linux
sudo apt install cmake make g++ libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libgles2

# macOS
brew install cmake sdl2 sdl2_image sdl2_mixer
```

### WebAssembly

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh
```

## Build

All commands are run from the `jpengine-2d/` directory.

### Native

```bash
make build                  # debug build (default game)
make build GAME=tetris      # debug build for a specific game
make run                    # build and run
make release                # optimized release build
```

### WebAssembly

```bash
make wasm                   # build for browser (default game: platformer)
make wasm GAME=tetris       # build a specific game
make wasm-run               # build and open in browser with emrun
make wasm-run GAME=tetris
```

Output lands in `bin/`: `main.html`, `main.js`, `main.wasm`, `main.data`.

## Project structure

```
jpengine-2d/
├── src/             # Engine C++ sources
├── include/         # Engine C++ headers
│   ├── ecs/         # Entity, Registry, components
│   ├── rendering/   # Shaders, textures, batch renderer
│   ├── inputs/      # Keyboard, mouse, gamepad
│   ├── scripting/   # Lua binding helpers
│   ├── sounds/      # Music and sound players
│   └── utils/       # Asset manager, utilities
├── games/           # One folder per game
│   ├── platformer/
│   │   └── assets/
│   │       ├── scripts/main.lua   # game entry point
│   │       ├── textures/
│   │       ├── fonts/
│   │       ├── music/
│   │       └── soundfx/
│   └── tetris/
│       ├── assets/
│       │   └── scripts/main.lua
│       └── web/
│           ├── shell.html         # custom browser shell
│           └── images/            # images referenced by shell.html
├── vendor/          # Bundled third-party libraries
│   ├── entt/
│   ├── glm/
│   ├── lua/
│   ├── sol2/
│   └── stb/
├── bin/             # Build output
└── CMakeLists.txt
```

## Adding a new game

1. Create `games/<name>/assets/scripts/main.lua` with a `main.update` function.
2. Add any textures, fonts, music, and sound effects under `games/<name>/assets/`.
3. Optionally add `games/<name>/web/shell.html` for a custom browser shell and `games/<name>/web/images/` for web assets.
4. Build with `make wasm GAME=<name>`.

## Lua API

Type stubs for every global and usertype live in `engine-defs/` (loaded by Lua LSP via `.luarc.json`). High-level overview:

### ECS

| Global | Description |
|--------|-------------|
| `Entity()` / `Entity(registry)` | Create or wrap an entity (`add_component`, `get_component`, `has_component`, `remove_component`, `destroy`, `id`) |
| `Registry()` | ECS registry (`create_entity`, `get_entities(...)` → `runtime_view`) |

### Components

| Component | Purpose |
|-----------|---------|
| `Transform` | Position, scale, rotation |
| `Sprite` | Textured quad (layer, UVs, tint, hidden) |
| `TextComponent` | Rendered text (font, color, hidden) |
| `Identification` | `tag`, `group`, `entity_id` |
| `RigidBody` | Simple velocity + max-velocity component |
| `BoxCollider` | AABB collider (`width`, `height`, `offset`, `active`) |
| `CircleCollider` | Radius collider (`radius`, `offset`, `trigger`, `active`) |
| `Animation` | Spritesheet animation (frames, rate, looped) |
| `PhysicsComp` | Box2D-backed body — built from a `PhysicsAttributes` table; supports `set/get_linear_velocity`, `set/get_angular_velocity`, `linear_impulse`, `angular_impulse`, `set/get_gravity_scale`, `set_transform`, `set_body_type`, `set_bullet`, `cast_ray(p1, p2)`, `box_trace(lower, upper)`, `object_data()` |

Physics helpers: `PhysicsAttributes{...}` (table-constructible), `ObjectData{tag, group, collider, trigger, is_friendly, entity_id}`, `BodyType.Static / Kinematic / Dynamic`.

### Rendering

| Global | Description |
|--------|-------------|
| `Color(r, g, b, a)` | Color value; presets: `J2D_WHITE`, `J2D_BLACK`, `J2D_RED`, `J2D_GREEN`, `J2D_BLUE`, `J2D_YELLOW`, `J2D_MAGENTA` |
| `UV(u, v, w, h)` | Spritesheet sub-region |
| `Rect(pos, size, color, wireframe)` | Rectangle shape |
| `Circle(center, radius, color, segments, wireframe)` | Circle shape |
| `Triangle(pos, base, height, color, wireframe)` | Triangle shape |
| `Polygon(points, color, wireframe)` | Polygon shape (`points` is a table of `vec2`) |
| `Line(p1, p2, color)` | Line segment |
| `draw_rect` / `draw_circle` / `draw_triangle` / `draw_polygon` / `draw_line` | Submit a shape to the batch renderer for this frame |
| `Camera` | `get_position`, `set_position`, `get_scale`, `set_scale` |

### Audio

| Global | Description |
|--------|-------------|
| `MusicPlayer` | `play(name[, loops])`, `stop`, `pause`, `resume`, `set_volume(v)`, `is_playing` — call with `.` (no `self`) |
| `SoundPlayer` | `play(name[, loops, channel])`, `stop(channel)`, `set_volume(v, channel)`, `is_playing(channel)` — call with `.` (no `self`) |
| `AssetManager` | `add_texture`, `add_font`, `add_music`, `add_soundfx`, and matching `get_*` |

### Input

| Global | Description |
|--------|-------------|
| `Keyboard` | `pressed`, `just_pressed`, `just_released` — constants `KEY_A`..`KEY_Z`, `KEY_0`..`KEY_9`, `KEY_SPACE`, `KEY_ENTER`, `KEY_ESC`, `KEY_LEFT/RIGHT/UP/DOWN`, `KEY_F1`..`KEY_F12`, modifiers, numpad |
| `Mouse` | `pressed`, `just_pressed`, `just_released`, `screen_position`, `wheel_x/y`; buttons `LEFT_BTN`, `MIDDLE_BTN`, `RIGHT_BTN` |
| `Gamepad` | `is_gamepad_present`, `pressed`, `just_pressed`, `just_released`, `get_axis_position`, `get_hat_value`; buttons (`GP_BTN_A/B/X/Y`, `GP_BTN_BACK/GUIDE/START`, shoulders, sticks), d-pad (`DPAD_*`), axes (`AXIS_X1/Y1/X2/Y2/Z1/Z2`) |

### Utilities

| Global | Description |
|--------|-------------|
| `vec2(x, y)` | 2D vector (GLM binding) |
| `Timer()` | Stopwatch — `start`, `stop`, `pause`, `resume`, `is_running`, `is_paused`, `elapsed_ms`, `elapsed_sec` |
| `j2d_run_script(path)` | Load and execute another Lua file |
| `j2d_load_script_table(scripts)` | Load an ordered list of scripts |
| `J2D_GetTicks()` | Milliseconds since engine start |
| `j2d_measure_text(text, font)` | Measure rendered text width |
| `j2d_right_align_text(text, font, pos)` | Right-align x position |
| `j2d_center_align_text(text, font, pos)` | Center x position |
