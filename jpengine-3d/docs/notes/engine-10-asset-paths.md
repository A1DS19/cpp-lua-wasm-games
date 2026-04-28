# 10 — Asset Paths and File Loading

> Previous: [08 — Walkthrough](./engine-08-walkthrough.md) · Next: [11 — Textures](./engine-11-textures.md)

The original engine had no concept of "asset directory" — shaders lived in inline `R"(...)"` strings inside the game code. As soon as you want textures, models, or materials in files, you need a way to **find them at runtime regardless of where the binary was launched from**.

This note covers two small utilities that all the loading code (textures, materials, meshes) builds on:

- `JPENGINE_ASSETS_DIR` — a compile-time-baked absolute path to `assets/`.
- `utils::asset_path` / `utils::asset_path_str` — turn a relative asset path into a real one.
- `utils::read_text_file` / `utils::read_asset_text` — read a file's contents as a string.

---

## The cwd problem

Naive code does:

```cpp
auto src = read_text_file("shaders/basic.vert");
```

That works only if the program was launched from the project root, because relative paths resolve against the **current working directory**. Run the binary from `bin/` and `shaders/...` is suddenly `bin/shaders/...` which doesn't exist.

Three common fixes:

| Approach | Pros | Cons |
|---|---|---|
| `chdir(...)` to a known dir at startup | simple | breaks IDE debuggers / changes user expectations |
| Copy `assets/` next to the binary every build | portable distribution | slow rebuilds, doubles disk use |
| **Bake an absolute path into the binary at compile time** | zero runtime overhead, works everywhere | path is fixed for that build (rebuild if you move the project) |

For development, the third is by far the simplest. We use it.

---

## The CMake side

```cmake
set(JPENGINE_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
target_compile_definitions(main PRIVATE JPENGINE_ASSETS_DIR="${JPENGINE_ASSETS_DIR}")
```

`CMAKE_SOURCE_DIR` is the project root. The line `target_compile_definitions(... JPENGINE_ASSETS_DIR="...")` becomes a `-DJPENGINE_ASSETS_DIR="/abs/path/to/assets"` flag passed to the compiler. From the C++ side, the macro expands to a string literal:

```cpp
const char* path = JPENGINE_ASSETS_DIR;   // "/Users/.../assets"
```

The path is frozen into the binary the moment CMake configures. Move the project, rerun `cmake -S . -B build`, and the new path is baked in.

---

## `utils::asset_path`

```cpp
// include/utils/asset-path.hpp
#ifndef JPENGINE_ASSETS_DIR
#error "JPENGINE_ASSETS_DIR is not defined. Configure with CMake."
#endif

namespace utils {
[[nodiscard]] inline std::filesystem::path asset_path(std::string_view rel) {
    return std::filesystem::path{JPENGINE_ASSETS_DIR} / rel;
}

[[nodiscard]] inline std::string asset_path_str(std::string_view rel) {
    return asset_path(rel).string();
}
}
```

Two small helpers:

- `asset_path("textures/wall.png")` → `std::filesystem::path` like `"/.../assets/textures/wall.png"`. Use this when you want to ask "does it exist?", "what's its extension?", etc.
- `asset_path_str("...")` → `std::string`. Use this when handing a path to a C API (`stbi_load`, `fopen`).

The `#error` guard makes it impossible to accidentally include this header in a TU that wasn't compiled through CMake — you'd otherwise get a confusing `JPENGINE_ASSETS_DIR is undeclared` from the function body.

Why two functions? `std::filesystem::path::string()` is **not implicit** — you have to call it. Returning `std::string` directly from `asset_path_str` saves one method call at every C-API call site:

```cpp
// without the helper:
stbi_load(asset_path("wall.png").string().c_str(), ...);
// with:
stbi_load(asset_path_str("wall.png").c_str(), ...);
```

---

## `utils::read_text_file` / `read_asset_text`

```cpp
// include/utils/file-utils.hpp
[[nodiscard]] std::string read_text_file(const std::filesystem::path& path);
[[nodiscard]] std::string read_asset_text(std::string_view relative_path);
```

Both throw `std::runtime_error` on failure. From the implementation:

```cpp
std::string read_text_file(const std::filesystem::path& path) {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        throw std::runtime_error("failed to open file: " + path.string());
    }
    std::stringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

std::string read_asset_text(std::string_view relative_path) {
    return read_text_file(asset_path(relative_path));
}
```

The pattern: `read_asset_text` is for **paths inside the assets folder** (the common case). `read_text_file` takes any absolute path (rare — usually a user-picked file).

### Why throw

This codebase's error policy is: **exceptions for fatal load/init failures, asserts for programmer errors, return values per-frame**. A missing shader or material file at startup is fatal — there's no sensible fallback, the game can't run. `main` catches once at the top:

```cpp
int main() {
    try {
        // engine setup, run loop, teardown
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
```

Result: a missing file produces one clean error message and a non-zero exit code, instead of a segfault or silent black screen.

---

## Practical usage

Anywhere in the codebase that needs an asset:

```cpp
#include "utils/asset-path.hpp"
#include "utils/file-utils.hpp"

// Read a shader file:
std::string vert_src = utils::read_asset_text("shaders/basic.vert");

// Get a path to hand to a C API:
auto img_path = utils::asset_path_str("textures/wall.png");
auto* pixels  = stbi_load(img_path.c_str(), &w, &h, &c, 4);

// Check if a file exists before loading:
if (std::filesystem::exists(utils::asset_path("models/player.gltf"))) {
    // ...
}
```

---

## What could go wrong

1. **Forgot to reconfigure after moving the project.** `JPENGINE_ASSETS_DIR` is baked at *configure* time, not build time. If you move `jpengine-3d/` to a new disk location, `cmake --build` keeps using the old path. Fix: `rm -rf build && cmake -S . -B build`.

2. **Path with spaces.** CMake quotes the macro correctly, but if you write your own `#define JPENGINE_ASSETS_DIR /path with spaces`, the preprocessor splits at the space. Always use quotes (which the CMake form does for you).

3. **Including `asset-path.hpp` in a unit-test target that doesn't define the macro.** The `#error` will fire. Either pass `-DJPENGINE_ASSETS_DIR="..."` to the test target too, or have tests stub a fake assets directory.

4. **Confusing absolute paths in `.mat` / `.gltf` files with relative ones.** Files passed to `read_text_file` directly need full paths. Files passed to `read_asset_text` are relative to `assets/`. Always pick one convention per call site and document it.

---

## Distribution mode (later)

For shipping, you don't want absolute paths baked in — the user's path won't match yours. Two strategies for shipping:

```cmake
if(JPENGINE_RELEASE)
    target_compile_definitions(main PRIVATE JPENGINE_ASSETS_DIR="assets")  # relative
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:main>/assets)
endif()
```

That puts `assets/` next to the binary and uses a relative path. Or pack everything into the binary as embedded data. Worry about this when you ship; it's overkill for development.

---

## See also

- [11 — Textures](./engine-11-textures.md) — the first major user of `asset_path`.
- [12 — Loading meshes from glTF](./engine-12-mesh-loading.md) — uses both `asset_path` and `read_asset_text`.
- [14 — Roadmap](./engine-14-roadmap.md) — the original "shaders from disk" item that started this work.
