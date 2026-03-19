# SDL2 + OpenGL ES 3 Rendering — Study Guide

## The big picture

```
Your CPU code (C++)
  └── SDL2          — creates a window, GL context, loads images
  └── SDL2_image    — loads PNG/JPG into SDL_Surface pixels
  └── stb_image     — alternative: loads images directly to raw bytes
  └── OpenGL ES 3   — draws into that window using the GPU
        └── Shaders — small GPU programs that decide positions and colors
  └── Lua           — scripting via sol2 binding
```

Everything visible on screen is drawn by the GPU. The CPU only sends data and instructions to it.

---

## Platform headers

macOS has no native `GLESv2`. Use a platform guard:

```cpp
#ifdef __APPLE__
#  include <OpenGL/gl3.h>   // macOS: desktop OpenGL 3.x (same API as GLES3)
#else
#  include <GLES3/gl3.h>    // Linux/Android/Emscripten
#endif
```

In CMake, link conditionally:

```cmake
if(APPLE)
  find_package(OpenGL REQUIRED)
  target_link_libraries(main PRIVATE OpenGL::GL)
else()
  find_library(GLES3_LIB GLESv2 REQUIRED)
  target_link_libraries(main PRIVATE ${GLES3_LIB})
endif()
```

---

## Step 1 — SDL2 sets up the window

SDL2 handles the OS-level stuff: creating a window, receiving input events, and managing the OpenGL context.

**Order matters:** GL attributes must be set *before* creating the window, because the driver uses them to select a compatible framebuffer.

```cpp
// 1. Tell SDL we want OpenGL ES 3.0
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

// 2. Create the window (must come AFTER GL attributes)
p_window = SDL_CreateWindow("title", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, 800, 600,
                             SDL_WINDOW_OPENGL);  // <-- flag required

// 3. Create the GL context bound to that window
gl_context = SDL_GL_CreateContext(p_window);

// 4. Enable vsync (optional but recommended)
SDL_GL_SetSwapInterval(1);

// 5. Enable alpha blending
glEnable(GL_BLEND);
glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
```

The **GL context** is the object that holds all GPU state (which shader is active, what buffers exist, etc.). Without it, no GL calls work.

---

## Step 2 — Upload geometry (quad)

The GPU can't access normal CPU memory directly. You have to explicitly upload vertex data into a **VBO** (Vertex Buffer Object) and describe it with a **VAO**.

### Quad vertex layout (position + UV)

Each vertex stores 4 floats: `x, y, u, v`

```
(-0.5, 0.5) u=0,v=0         (0.5, 0.5) u=1,v=0
      v3 ┌───────────────────┐ v2
         │                   │
         │                   │
      v0 └───────────────────┘ v1
(-0.5,-0.5) u=0,v=1         (0.5,-0.5) u=1,v=1
```

UV is flipped vertically (0 at top, 1 at bottom) because image files store rows top-to-bottom but OpenGL's origin is bottom-left.

```cpp
// x      y      u     v
auto vertices = std::array<float, 16>{
    -0.5f, -0.5f,  0.0f, 1.0f,  // v0 bottom-left
     0.5f, -0.5f,  1.0f, 1.0f,  // v1 bottom-right
     0.5f,  0.5f,  1.0f, 0.0f,  // v2 top-right
    -0.5f,  0.5f,  0.0f, 0.0f,  // v3 top-left
};

// Two triangles forming a quad: (v0,v1,v2) + (v0,v2,v3)
auto indices = std::array<uint8_t, 6>{ 0, 1, 2,  0, 2, 3 };
```

### Uploading to the GPU

```cpp
glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);
glGenBuffers(1, &ebo);       // EBO = index buffer
glBindVertexArray(vao);

glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

constexpr int stride = 4 * sizeof(float);  // x, y, u, v

// attribute 0 = position (x, y)
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
glEnableVertexAttribArray(0);

// attribute 1 = uv (u, v) — offset by 2 floats
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
glEnableVertexAttribArray(1);
```

### EBO / index type must match

`indices` is `uint8_t` → use `GL_UNSIGNED_BYTE` in the draw call:

```cpp
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
//                               ^^^^^^^^^^^^^^^^ must match the C++ type
```

| C++ type | GL constant |
|---|---|
| `uint8_t` | `GL_UNSIGNED_BYTE` |
| `uint16_t` | `GL_UNSIGNED_SHORT` |
| `uint32_t` | `GL_UNSIGNED_INT` |

---

## Step 3 — Shaders

Shaders are small programs that run on the GPU, written in **GLSL ES**.

Two shaders are always required:

```
For each vertex ──► Vertex shader   — WHERE on screen does this point go?
For each pixel  ──► Fragment shader — WHAT COLOR is this pixel?
```

### Vertex shader (with UV pass-through)

```glsl
#version 300 es
in vec2 a_pos;       // attribute slot 0 — position
in vec2 a_uvs;       // attribute slot 1 — UV coords
out vec2 frag_uvs;   // passed to fragment shader

void main() {
    gl_Position = vec4(a_pos, 0.0, 1.0);
    frag_uvs = a_uvs;
}
```

### Fragment shader (texture sampling)

```glsl
#version 300 es
precision mediump float;
in vec2 frag_uvs;
out vec4 fragColor;
uniform sampler2D u_texture;   // ← sampler2D, NOT sample2D

void main() {
    fragColor = texture(u_texture, frag_uvs);
}
```

### GLSL ES rules (strict — will break silently in WebGL if ignored)

| Rule | Wrong | Correct |
|---|---|---|
| `#version` must be line 1 | blank line before it | `#version 300 es` at top |
| No `f` suffix on floats | `1.0f` | `1.0` |
| Precision required in frag | _(missing)_ | `precision mediump float;` |
| Texture type | `sample2D` | `sampler2D` |

### Compiling shaders at runtime

```
glCreateShader(GL_VERTEX_SHADER)          — allocate shader object on GPU
glShaderSource(shader, 1, &src, nullptr)  — upload GLSL source text
glCompileShader(shader)                   — driver compiles it (check glGetShaderInfoLog)

glCreateProgram()                         — allocate the final linked program
glAttachShader(program, vert)
glAttachShader(program, frag)
glLinkProgram(program)                    — link vert + frag (check glGetProgramInfoLog)
```

Shader compilation happens **at runtime** on the GPU driver — not at build time. Always check for errors after compile and link.

---

## Step 4 — Loading textures

### With SDL2_image + OpenGL

```cpp
#include <SDL2/SDL_image.h>

GLuint load_texture(const std::string_view file_name, bool pixel_art,
                    int& width, int& height) {
    GLuint texture_id{0};
    SDL_Surface* surface = IMG_Load(file_name.data());
    if (!surface) return 0;

    // Normalize pixel format
    SDL_Surface* fmt = (surface->format->BytesPerPixel == 3)
        ? SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB24, 0)
        : SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);

    GLenum format = (surface->format->BytesPerPixel == 3) ? GL_RGB : GL_RGBA;
    width  = fmt->w;
    height = fmt->h;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);   // ← GL_TEXTURE_2D, not GL_TEXTURE_BINDING_2D
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                 format, GL_UNSIGNED_BYTE, fmt->pixels);

    GLenum filter = pixel_art ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    SDL_FreeSurface(fmt);
    SDL_FreeSurface(surface);
    return texture_id;
}
```

**Common mistake:** `GL_TEXTURE_BINDING_2D` is a *query* constant (used with `glGetIntegerv`), not a bind target. Using it in `glBindTexture` produces `INVALID_ENUM`.

### stb_image

For loading pixels directly (without SDL surface overhead):

```cpp
// vendor/stb/src/stb_impl.cpp — exactly ONE file defines the implementation
#ifndef __EMSCRIPTEN__
#  define STB_IMAGE_IMPLEMENTATION
#  include <stb_image.h>
#endif
// Emscripten's SDL2_image port ships its own stb_image — don't redefine it
```

```cpp
#include <stb_image.h>

int w, h, channels;
unsigned char* data = stbi_load("assets/texture.png", &w, &h, &channels, 4);
// upload to GL with glTexImage2D ...
stbi_image_free(data);
```

---

## Step 5 — The game loop

Every frame follows the same sequence:

```
┌─────────────────────────────────────────────────────┐
│ 1. Poll SDL events   (input, window close)          │
│ 2. Clear the screen                                 │
│ 3. Set viewport      (map clip space to pixels)     │
│ 4. glUseProgram      (activate shaders)             │
│ 5. glBindVertexArray (activate geometry)            │
│ 6. glDrawElements    (GPU runs shaders → pixels)    │
│ 7. SDL_GL_SwapWindow (flip back buffer to screen)   │
└─────────────────────────────────────────────────────┘
```

```cpp
glClearColor(0.0, 0.0, 0.0, 1.0);
glClear(GL_COLOR_BUFFER_BIT);

int w, h;
SDL_GetWindowSize(p_window, &w, &h);
glViewport(0, 0, w, h);

glUseProgram(shader_program);
glBindVertexArray(vao);
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);

SDL_GL_SwapWindow(p_window);
```

**Double buffering:** you draw into a hidden back buffer, then `SwapWindow` flips it to screen atomically so there's no tearing.

---

## WebAssembly (Emscripten) differences

### Main loop

The browser's JS event loop cannot be blocked. `while(true)` would freeze the tab entirely.

```cpp
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void game_loop() { /* ... */ }

// In main():
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(game_loop, 0, 1);
    // 0 = use requestAnimationFrame (~60fps)
    // 1 = simulate_infinite_loop (prevents code after this from running)
#else
    while (true) { game_loop(); }
#endif
```

To stop the loop:
```cpp
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
```

### WebGL2 = OpenGL ES 3

WebGL2 exposes the same GLES3 API. Enable it in CMake link flags:
```
-sUSE_WEBGL2=1 -sFULL_ES3=1
```

### SDL2 + SDL2_image ports

Emscripten bundles both as built-in ports — no system library needed:

```cmake
if(EMSCRIPTEN)
  target_compile_options(main PRIVATE -sUSE_SDL=2 -sUSE_SDL_IMAGE=2)
  target_link_options(main PRIVATE
    -sUSE_SDL=2
    -sUSE_SDL_IMAGE=2
    -sSDL2_IMAGE_FORMATS=["png","jpg"]
    -sALLOW_MEMORY_GROWTH=1
    -sUSE_WEBGL2=1
    -sFULL_ES3=1
  )
endif()
```

### Asset loading

The browser has no real filesystem. Files are bundled into a virtual FS at build time:
```
--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets
```
This packages the `assets/` folder so textures and scripts can be loaded with the same relative paths at runtime.

### Exceptions

Emscripten disables C++ exceptions by default. sol2 (Lua binding) requires them. Enable wasm-native exceptions:
```
-fwasm-exceptions   (compile + link flags, applied to both main and lua targets)
```

### Summary table

| Topic | Native macOS/Linux | WASM |
|---|---|---|
| OpenGL header | `OpenGL/gl3.h` (mac) / `GLES3/gl3.h` (linux) | auto via `-sUSE_WEBGL2=1` |
| SDL2 / SDL2_image | system / Homebrew | Emscripten port (`-sUSE_SDL=2`) |
| stb_image impl | `stb_impl.cpp` defines it | skipped (`#ifndef __EMSCRIPTEN__`) |
| Main loop | `while(true)` | `emscripten_set_main_loop` |
| Assets | relative paths | `--preload-file` virtual FS |
| Exceptions | normal | `-fwasm-exceptions` |
| GLSL floats | `1.0f` tolerated | `1.0f` rejected — use `1.0` |
| `#version` position | flexible | must be line 1, no leading newline |
