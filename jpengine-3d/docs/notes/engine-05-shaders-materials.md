# 05 — Shaders & Materials

> Previous: [04 — Graphics API](./engine-04-graphics-api.md) · Next: [06 — Meshes](./engine-06-meshes.md)

Two ideas in this doc:

- **`ShaderProgram`** — a C++ handle for a linked GPU shader.
- **`Material`** — a shader + the parameter values you want to feed it.

If you know what [a shader is](./graphics-pipeline.md), you can think of `ShaderProgram` as "the compiled code" and `Material` as "the recipe that uses that code."

---

## `ShaderProgram`

### Why wrap the GL id?

When you compile and link a GLSL shader, OpenGL gives you back a number — a `GLuint` — that identifies the program. You pass it to `glUseProgram`, `glGetUniformLocation`, etc. That's it. No type, no safety, no cleanup.

Wrapping the id in a C++ class buys you three things:

1. **RAII.** The destructor calls `glDeleteProgram`. No leaks when the object goes out of scope.
2. **Encapsulation.** Other code uses `shader.bind()` instead of remembering `glUseProgram(id)`.
3. **Caching.** Looking up uniform locations by name is slow (GL does a string search); caching them on the program handle makes repeated `set_uniform` calls cheap.

### The class

`include/engine/src/graphics/shader-program.hpp`:

```cpp
namespace engine {

class ShaderProgram {
public:
    ShaderProgram() = delete;
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&&) = delete;

    explicit ShaderProgram(GLuint id) : shader_program_id_{id} {}
    ~ShaderProgram() { glDeleteProgram(shader_program_id_); }

    void bind();
    GLint get_uniform_location(const std::string& name);
    void set_uniform(const std::string& name, float value);

private:
    std::unordered_map<std::string, GLint> uniform_location_cache_;
    GLuint shader_program_id_ = 0;
};

}
```

**Non-copyable by design.** If you copied a `ShaderProgram`, both copies would share the same `shader_program_id_`, and when the first one got destroyed it would `glDeleteProgram` the id out from under the second. A classic C++ resource-ownership bug. Deleting the copy constructor and copy-assignment prevents that at compile time.

The public API:

| Method | What it does |
|--------|--------------|
| `bind()` | `glUseProgram(shader_program_id_)`. Makes this program current for subsequent draws. |
| `get_uniform_location(name)` | Returns the GL location of a uniform, cached after first lookup. |
| `set_uniform(name, float)` | Looks up the location (cached) and uploads a float value. |

### `bind()` implementation

```cpp
void ShaderProgram::bind() {
    glUseProgram(shader_program_id_);
}
```

Once called, every draw call until the next `glUseProgram` uses this program. Shaders are global GL state — there is exactly one program bound at a time.

(An earlier bug: `bind()` was an empty function body. The whole material/mesh stack wired up correctly, but nothing ever called `glUseProgram`, so `glDrawElements` ran with no program bound and drew nothing. The fix was a one-liner — but it took walking through the render-queue pipeline in [doc 08](./engine-08-walkthrough.md) to find.)

### Uniform location cache

```cpp
GLint ShaderProgram::get_uniform_location(const std::string& name) {
    auto it = uniform_location_cache_.find(name);
    if (it != uniform_location_cache_.end()) return it->second;

    GLint location = glGetUniformLocation(shader_program_id_, name.c_str());
    uniform_location_cache_[name] = location;
    return location;
}
```

`glGetUniformLocation` scans the program's symbol table for the name. Doing it every frame for every uniform would be wasteful. The cache turns it into an `unordered_map` lookup after the first call.

**Gotcha:** if the name doesn't exist in the shader (typo, optimized out), `glGetUniformLocation` returns `-1`. That's not an error — `glUniform*` with location `-1` is silently ignored. The cache stores the `-1` so you don't ask again.

### `set_uniform`

```cpp
void ShaderProgram::set_uniform(const std::string& name, float value) {
    auto location = get_uniform_location(name);
    glUniform1f(location, value);
}
```

Only a `float` overload today. Shader parameters that aren't a single float (positions, colors, matrices) can't be uploaded through `Material` yet.

(Another earlier bug: `glUniform1d` was used here. `glUniform1d` is for doubles and requires `GL_ARB_gpu_shader_fp64` — on a GL 3.3 core context the function pointer may not even load, and calling it against a `float` uniform produces `GL_INVALID_OPERATION`. Nothing crashes; the uniform just never gets the value. Moral: match the `glUniform*` suffix to your shader's uniform type.)

---

## `Material`

### Why this exists

A shader alone doesn't draw a *thing*. It draws *whatever geometry you feed it, using whatever parameter values you upload*. If two meshes use the same shader with different parameters (red vs blue, shiny vs matte), you need to:

- bind the shader,
- upload parameters A, draw mesh 1,
- upload parameters B, draw mesh 2.

**Material** is the abstraction for "parameters A" and "parameters B." It bundles a shader pointer with a bag of parameter values. Hitting the same shader with different materials is the common case.

### The class

`include/engine/src/render/material.hpp`:

```cpp
namespace engine {

class ShaderProgram;

class Material {
public:
    void set_shader_program(const std::shared_ptr<ShaderProgram>& p) {
        pshader_program_ = p;
    }
    void set_param(const std::string& name, float value) {
        float_params_[name] = value;
    }
    void bind();

private:
    std::shared_ptr<ShaderProgram>           pshader_program_;
    std::unordered_map<std::string, float>   float_params_;
};

}
```

Two pieces of state:

- **`pshader_program_`** — shared ownership of the shader. Many materials can share one shader; the shader lives as long as any material using it.
- **`float_params_`** — key → value for uniform floats.

### `Material::bind()`

`src/engine/src/render/material.cpp`:

```cpp
void Material::bind() {
    if (!pshader_program_) return;

    pshader_program_->bind();

    for (auto& [name, value] : float_params_) {
        pshader_program_->set_uniform(name, value);
    }
}
```

Binding a material is two steps:

1. Bind the shader program (`glUseProgram`).
2. For each stored param, upload it to the corresponding uniform.

Ordering matters: `glUniform*` only works on the **currently bound** program. If you tried to set uniforms before binding, nothing would happen (or, worse, they'd end up on whatever program was bound previously).

### Using a material

```cpp
// set up once
pshader_program_ = graphics_api.create_shader_program(vs, fs);
material_.set_shader_program(pshader_program_);

// every frame, before submitting the draw
material_.set_param("offset_x", offset_x);
material_.set_param("offset_y", offset_y);
```

You don't call `material_.bind()` yourself — the [render queue](./engine-07-render-queue.md) does that when it drains its commands.

---

## Shaders in strings — the current approach

Right now the shader source is a raw string literal in `Game::init`:

```cpp
const std::string vertex_source = R"(
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
)";
```

Pros: shaders ship in the binary; no file I/O; great for a minimal demo.

Cons: editing a shader means rebuilding the engine. No live reload. No syntax highlighting from your editor (the shader is just a C++ string).

The [roadmap](./engine-09-roadmap.md) mentions loading shaders from disk, which is the usual next step.

---

## What's naive about this

- **Only float uniforms.** No vec2/vec3/vec4, no `mat4`, no textures. You can't implement a camera (needs `mat4`) or a textured quad (needs `sampler2D`) with the current `Material`. This is the most important gap.
- **No type checking.** Nothing verifies the shader actually has a uniform with the name you're setting. A typo in `set_param("offest_x", v)` fails silently.
- **No parameter inheritance.** If you want "all materials share these ambient-light params," you'd copy them into every material instance by hand.
- **No shader permutations.** Real engines compile the same shader source with different `#define`s (lit vs unlit, skinned vs static). Here you'd have to duplicate the source.
- **Shader sources are string literals.** No hot reload.

---

## See also

- [04 — Graphics API](./engine-04-graphics-api.md) — where `create_shader_program` lives.
- [07 — Render queue](./engine-07-render-queue.md) — who actually calls `material.bind()`.
- [08 — Walkthrough](./engine-08-walkthrough.md) — the full shader source for the rectangle demo.
- [vao-vbo-shaders.md](./vao-vbo-shaders.md) — background on what a shader program is.
- [graphics-pipeline.md](./graphics-pipeline.md) — the vertex/fragment stages.
