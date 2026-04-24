# 04 — The Graphics API Wrapper

> Previous: [03 — Input](./engine-03-input.md) · Next: [05 — Shaders & Materials](./engine-05-shaders-materials.md)

## Why this exists

OpenGL is a **global state machine driven by a C API**. To draw one shaded mesh you end up writing 30–50 lines of `glGenBuffers`, `glBindBuffer`, `glBufferData`, `glCreateShader`, `glShaderSource`, `glCompileShader`, `glCreateProgram`, `glAttachShader`, `glLinkProgram`, `glVertexAttribPointer`, `glEnableVertexAttribArray`, `glUseProgram`, `glDrawElements`, … and every one of them fails silently if you get the order wrong.

Three specific problems show up when raw GL is scattered across gameplay code:

1. **Boilerplate.** The same "compile shader" sequence everywhere.
2. **Error reporting.** GL doesn't throw; you have to query `glGetShaderiv(...)`, pull an info log, and log it yourself — every time.
3. **Coupling.** If every file uses GL directly, you can never swap to a different backend (Vulkan, Metal, WebGPU) without rewriting the world.

A **graphics API wrapper** is a thin layer that:
- turns "compile+link a shader program" into one function call that returns a handle,
- centralises error reporting,
- gives the rest of the engine a stable API so the GL internals can change later.

`GraphicsApi` is jpengine-3d's wrapper. It's deliberately small right now — just enough for the current rendering features.

---

## The interface

`include/engine/src/graphics/graphics-api.hpp`:

```cpp
namespace engine {

class ShaderProgram;
class Material;
class Mesh;

class GraphicsApi {
public:
    std::shared_ptr<ShaderProgram>
    create_shader_program(const std::string& vertex_source,
                          const std::string& fragment_source);

    void bind_shader_program(ShaderProgram* pshader_program);
    void bind_material(Material* pmaterial);
    void bind_mesh(Mesh* pmesh);
    void draw_mesh(Mesh* pmesh);

    uint32_t create_vertex_buffer(const std::vector<float>& vertices);
    uint32_t create_index_buffer(const std::vector<uint32_t>& indices);
};

}
```

Four categories of method:

| Category        | Methods                                           | What they do                                  |
|-----------------|---------------------------------------------------|-----------------------------------------------|
| Create resources| `create_shader_program`, `create_vertex_buffer`, `create_index_buffer` | allocate GL objects, upload data, return a handle |
| Bind resources  | `bind_shader_program`, `bind_material`, `bind_mesh` | make a resource "current" for drawing         |
| Issue draws     | `draw_mesh`                                       | actually render something                      |

**No `glClear`, no `glViewport`, no `glEnable`.** Those still happen in `Engine::run` directly. The wrapper isn't trying to hide *all* of GL yet — just the high-value, high-repetition parts.

---

## Why the header no longer includes GL

The public API signatures use `uint32_t` (from `<cstdint>`), `std::string`, `std::vector`, and `std::shared_ptr`. It does **not** include `<GL/glew.h>` or any OpenGL header.

This is deliberate:

- **Headers that include GL leak types into every translation unit that includes them.** Compile times balloon.
- **Including GLFW's private `internal.h` for GL types** (which an early version of this header did) caused redefinition errors against GLEW's `<GL/glew.h>`. The two both define `GL_NUM_EXTENSIONS`, `GL_CONTEXT_FLAGS`, etc. You get an hour-long debugging session the first time you see it.
- **`GLuint` is just a typedef for `unsigned int`.** You don't lose anything using `uint32_t` at the interface — it's the same type underneath, but portable and zero-include.

Rule: GL types stay inside `.cpp` files. Public headers use `uint32_t` or `unsigned int`.

---

## Shader creation

`src/engine/src/graphics/graphics-api.cpp` (condensed):

```cpp
namespace {

GLuint compile_shader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::string log(static_cast<std::size_t>(length), '\0');
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        std::cerr << "shader compile failed: " << log << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

} // namespace

std::shared_ptr<ShaderProgram>
GraphicsApi::create_shader_program(const std::string& vertex_source,
                                   const std::string& fragment_source) {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_source);
    if (vs == 0) return nullptr;

    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    if (fs == 0) { glDeleteShader(vs); return nullptr; }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        // log and clean up
        return nullptr;
    }

    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return std::make_shared<ShaderProgram>(program);
}
```

The flow:

1. Compile the vertex shader. If that fails, give up.
2. Compile the fragment shader. If that fails, delete the vertex shader and give up.
3. Link the two into a program. If linking fails, delete everything and give up.
4. Once linked, you can **detach and delete** the individual shaders — the program retains the compiled code internally.
5. Return the program id wrapped in a `shared_ptr<ShaderProgram>`. `ShaderProgram`'s destructor calls `glDeleteProgram`, so lifetime is automatic.

**The anonymous-namespace helper** is a common C++ pattern for file-local functions. `namespace { ... }` is the modern equivalent of `static`; it prevents the helper from leaking into other translation units.

**Failure mode:** if either compile or link fails, the callable logs to `std::cerr` and returns `nullptr`. Callers check for nullptr to know something went wrong (`Game::init` returns `pshader_program_ != nullptr`).

---

## Buffer creation

```cpp
uint32_t GraphicsApi::create_vertex_buffer(const std::vector<float>& vertices) {
    GLuint buffer_id = 0;
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(),
                 GL_STATIC_DRAW);
    return buffer_id;
}

uint32_t GraphicsApi::create_index_buffer(const std::vector<uint32_t>& indices) {
    GLuint buffer_id = 0;
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)),
                 indices.data(),
                 GL_STATIC_DRAW);
    return buffer_id;
}
```

Three things to notice:

- **`glGenBuffers` + `glBindBuffer` + `glBufferData`** — the canonical "make a buffer, put data in it" sequence. See [vao-vbo-shaders.md](./vao-vbo-shaders.md).
- **Different targets for the two buffers.** Vertex data → `GL_ARRAY_BUFFER`. Indices → `GL_ELEMENT_ARRAY_BUFFER`. GL doesn't care what's in the buffer bytes; the target determines what GL uses them for.
- **Index type is `uint32_t`**, not `float`. An earlier draft of the API had `std::vector<float>` for indices — which compiles but produces garbage at draw time because `glDrawElements(..., GL_UNSIGNED_INT, ...)` reinterprets the float bytes as ints. Match the type to the draw-time format.

`GL_STATIC_DRAW` is a **usage hint** to the driver: "I'll upload this once and draw from it many times." The driver uses this to pick a good memory location (usually VRAM). Other options are `GL_DYNAMIC_DRAW` (update often) and `GL_STREAM_DRAW` (update every frame).

---

## Bind helpers

```cpp
void GraphicsApi::bind_shader_program(ShaderProgram* p) { if (p) p->bind(); }
void GraphicsApi::bind_material(Material* p)            { if (p) p->bind(); }
void GraphicsApi::bind_mesh(Mesh* p)                    { if (p) p->bind(); }
void GraphicsApi::draw_mesh(Mesh* p)                    { if (p) p->draw(); }
```

These are one-liners that forward to the object's own `bind()` / `draw()` method, with a null guard. They look pointless — why not call `material->bind()` directly?

Two reasons:

1. **Symmetry.** When the `RenderQueue` walks its commands, it calls a uniform API (`graphics_api.bind_X`) regardless of what the command's content is. You could later add logging, state caching, or backend dispatch without touching callers.
2. **Backend abstraction.** If you ever swap GL for another backend, you'd reimplement these methods to speak to the other API. The rest of the engine doesn't have to know.

At the current scale it's a very thin shell. That's fine — premature abstraction is worse than simple forwarding.

---

## Why this isn't a "real" abstraction layer

`GraphicsApi` is **not** a rendering abstraction in the RHI sense (Render Hardware Interface — think bgfx, wgpu, Sokol). A real RHI would:

- Hide GL enums behind portable ones (`PixelFormat::Rgba8`, not `GL_RGBA`).
- Manage resource lifetimes (handles you trade in, not raw IDs).
- Describe pipeline state (shader + blend + depth + cull) as a single object.

jpengine-3d is OpenGL-specific and makes no attempt to hide that. The wrapper exists to cut GL boilerplate, not to let you port to Vulkan tomorrow.

---

## What's naive about this

- **Everything's a public method on one class.** As the API grows this will get crowded. Eventually you'd split it — a `ResourceManager` for creates, a `Renderer` for draws.
- **No error batching.** Each create logs independently; you can't get a summary.
- **No resource handles.** `uint32_t buffer_id` is a raw GL id. If you delete it, any other code holding that id doesn't know. A proper handle system would use typed wrappers that delete on destruction.
- **Bind helpers don't track state.** If you call `bind_material(m)` twice with the same material, GL gets two `glUseProgram` calls. A real renderer caches the last binding and skips redundant calls.

---

## See also

- [05 — Shaders & Materials](./engine-05-shaders-materials.md) — what `ShaderProgram` and `Material::bind()` actually do.
- [06 — Meshes](./engine-06-meshes.md) — how `Mesh` consumes the buffer ids from `create_*_buffer`.
- [vao-vbo-shaders.md](./vao-vbo-shaders.md) — background on VBO/EBO creation.
- [graphics-pipeline.md](./graphics-pipeline.md) — what happens between "bind a shader" and "pixel on screen."
