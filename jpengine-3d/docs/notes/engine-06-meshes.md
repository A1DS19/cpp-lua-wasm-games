# 06 — Meshes

> Previous: [05 — Shaders & Materials](./engine-05-shaders-materials.md) · Next: [07 — Render queue](./engine-07-render-queue.md)

A **mesh** is "a hunk of geometry you want to draw." Internally that means a VBO (vertex data), an optional EBO (indices), a VAO (the recipe that points at them), and a description of how bytes in the VBO map to shader inputs.

The engine groups all of that into two things:

- **`VertexLayout`** — pure data: "each vertex is N bytes; here's how to read attributes out of it."
- **`Mesh`** — the GL objects and the draw call, driven by a `VertexLayout`.

If you haven't read [vao-vbo-shaders.md](./vao-vbo-shaders.md), do that first — this doc assumes you know what a VAO/VBO/EBO are.

---

## `VertexLayout`

### Why this exists

The only way OpenGL learns about your vertex format is through `glVertexAttribPointer` calls:

```cpp
glVertexAttribPointer(
    0,                    // attribute location in the shader
    3,                    // number of components (vec3)
    GL_FLOAT,             // component type
    GL_FALSE,             // normalize?
    6 * sizeof(float),    // stride — total bytes per vertex
    (void*)0              // offset — bytes from start of vertex
);
glEnableVertexAttribArray(0);
```

One call per attribute. For a vertex with position + color + normal + UV you have four of these. They're imperative, error-prone, and scattered across creation code.

`VertexLayout` is the same information **as data**:

```cpp
engine::VertexLayout layout{
    .elements_ = {
        {.index_ = 0, .size_ = 3, .type_ = GL_FLOAT, .offset_ = 0},
        {.index_ = 1, .size_ = 3, .type_ = GL_FLOAT,
         .offset_ = static_cast<uint32_t>(3 * sizeof(float))},
    },
    .stride_ = static_cast<uint32_t>(6 * sizeof(float)),
};
```

The mesh constructor reads this and issues the `glVertexAttribPointer` calls for you. You describe the layout once; the imperative calls happen in one place.

### The types

`include/engine/src/graphics/vertex-layout.hpp`:

```cpp
namespace engine {

struct VertexElement {
    uint32_t index_;    // attribute location (matches `layout(location = N)` in shader)
    uint32_t size_;     // number of components (1, 2, 3, or 4)
    uint32_t type_;     // data type (e.g. GL_FLOAT)
    uint32_t offset_;   // byte offset from the start of a vertex
};

struct VertexLayout {
    std::vector<VertexElement> elements_;
    uint32_t stride_ = 0;    // total size of a single vertex in bytes
};

}
```

**All four fields are `uint32_t`.** `GLuint` is also 32-bit unsigned, but using plain `uint32_t` keeps GL out of the public header (see [doc 04](./engine-04-graphics-api.md)).

**`type_` stores a GL enum** (`GL_FLOAT = 0x1406`, etc.). That's a small leakage — the layout is supposedly GL-agnostic but these enum values are GL's. A more purist design would define its own `VertexType::Float32` and translate it to `GL_FLOAT` inside `Mesh`. For now the cost (one `#include <GL/glew.h>` in `game.cpp`) is accepted in exchange for simplicity.

---

## Interleaved vs non-interleaved layouts

There are two ways to arrange vertex data:

**Interleaved** (what this engine does):

```
[ pos.x pos.y pos.z  color.r color.g color.b ]  ← vertex 0
[ pos.x pos.y pos.z  color.r color.g color.b ]  ← vertex 1
...
```

- stride = size of one vertex (6 × 4 bytes = 24).
- position offset = 0, color offset = 12.
- **One VBO** holds everything.

**Separate arrays**:

```
positions:  [ x y z  x y z  x y z  x y z ]    ← one VBO
colors:     [ r g b  r g b  r g b  r g b ]    ← another VBO
```

- stride in each = size of one attribute.
- offset in each = 0.
- **Multiple VBOs.**

Interleaved is usually better: the GPU reads one vertex's data from contiguous memory, which is friendlier to caches. Separate arrays are useful when you often update one attribute without touching others.

`VertexLayout` supports both because the *stride* field is the user's responsibility — you set whatever stride matches your data.

---

## `Mesh`

### Responsibility

Given a `VertexLayout`, some vertex data, and optionally some indices, produce a drawable thing.

`include/engine/src/render/mesh.hpp`:

```cpp
namespace engine {

class Mesh {
public:
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(const VertexLayout& layout,
         const std::vector<float> vertices,
         const std::vector<uint32_t> indices);

    Mesh(const VertexLayout& layout,
         const std::vector<float> vertices);       // no indices

    void bind();
    void draw();

private:
    VertexLayout vertex_layout_;
    size_t  vertex_count_ = 0;
    size_t  index_count_  = 0;
    GLuint  vbo_ = 0;
    GLuint  ebo_ = 0;
    GLuint  vao_ = 0;
};

}
```

Non-copyable, same reasoning as [`ShaderProgram`](./engine-05-shaders-materials.md) — copying would duplicate the GL ids and the destructor would double-delete.

Two constructors:

- **With indices** → creates VBO + EBO + VAO. `draw()` uses `glDrawElements`.
- **Without indices** → creates VBO + VAO, no EBO. `draw()` uses `glDrawArrays`.

### Constructor flow (indexed version)

`src/engine/src/render/mesh.cpp`:

```cpp
Mesh::Mesh(const VertexLayout& layout,
           const std::vector<float> vertices,
           const std::vector<uint32_t> indices) {
    vertex_layout_ = layout;
    auto& graphics_api = engine::Engine::get_instance().get_graphics_api();

    vbo_ = graphics_api.create_vertex_buffer(vertices);
    ebo_ = graphics_api.create_index_buffer(indices);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    for (auto& element : vertex_layout_.elements_) {
        glVertexAttribPointer(
            element.index_,
            static_cast<GLint>(element.size_),
            element.type_,
            GL_FALSE,
            static_cast<GLsizei>(vertex_layout_.stride_),
            reinterpret_cast<void*>(static_cast<uintptr_t>(element.offset_)));
        glEnableVertexAttribArray(element.index_);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    vertex_count_ = (vertices.size() * sizeof(float)) / vertex_layout_.stride_;
    index_count_  = indices.size();
}
```

Step by step:

1. **Upload the buffers** via `GraphicsApi` (doc 04). `vbo_` and `ebo_` are the returned handles.
2. **Create and bind the VAO.** A VAO is a recording device — while it's bound, certain GL state changes get saved *into* the VAO.
3. **Bind the VBO.** The VAO remembers which VBO was bound for each attribute.
4. **Issue `glVertexAttribPointer` calls for each element of the layout.** These go into the VAO's state.
5. **Bind the EBO** while the VAO is still bound. The VAO remembers the EBO binding too.
6. **Unbind the VAO first**, then the buffers. Order matters — unbinding the EBO while the VAO is bound would *erase* the EBO binding from the VAO. The fix is to always unbind the VAO first.
7. **Compute `vertex_count_`.** `vertices.size()` is a count of floats; dividing by stride-in-bytes is… wrong. The current code divides `(floats × 4 bytes)` by `stride_`. For a 6-float vertex that's `(n * 4) / 24 = n / 6`, which happens to give the right answer. It works by accident of arithmetic; cleaner would be `(vertices.size() * sizeof(float)) / vertex_layout_.stride_` — which is what the code does — but calling it out because the "`vertices.size()` in bytes divided by stride in bytes" intent is easy to lose.
8. **`index_count_ = indices.size()`** is straightforward.

### About that pointer cast

```cpp
reinterpret_cast<void*>(static_cast<uintptr_t>(element.offset_))
```

This looks odd — we're turning a `uint32_t` offset into a `void*`. That's a historical GL API quirk: the last parameter of `glVertexAttribPointer` was originally a client-side memory pointer. With VBOs, it's now interpreted as a **byte offset into the currently bound VBO**. But the function signature still wants a `void*`.

Two casts because going `uint32_t → void*` directly trips `-Wint-to-pointer-cast`; `uint32_t → uintptr_t → void*` is portable.

### `bind()` and `draw()`

```cpp
void Mesh::bind() {
    glBindVertexArray(vao_);
}

void Mesh::draw() {
    if (index_count_ > 0) {
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(index_count_),
                       GL_UNSIGNED_INT,
                       nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0,
                     static_cast<GLsizei>(vertex_count_));
    }
}
```

Binding the VAO reactivates everything we recorded into it during construction — VBO binding, attribute pointers, EBO binding. One call to bring it all back.

Drawing picks between `glDrawElements` (read indices from the EBO) and `glDrawArrays` (go straight through the VBO in order). Both interpret the vertices as triangles (`GL_TRIANGLES`) — every three vertices/indices becomes one triangle.

The `nullptr` passed to `glDrawElements` is the "where do indices start" argument. When an EBO is bound (which it is, via the VAO), `0`/`nullptr` means "start at the beginning of the EBO."

---

## What's naive about this

- **Vertex data is copied by value into the constructor.** `std::vector<float>` passed by value → copy. Clang-tidy warns about this. For a demo it's fine; for big meshes, change the signature to `const std::vector<float>&` or take an `std::span`/`std::vector<float>&&`.
- **No reupload / update.** Once constructed, you can't change the vertex data. If you needed a dynamic mesh (particles), you'd need a `Mesh::update(vertices)` method that re-uploads.
- **GL types leak into the header.** `Mesh::vbo_`, `ebo_`, `vao_` are `GLuint`, and the header includes `"glew/include/GL/glew.h"`. That's a public-API wart — users of `mesh.hpp` pay the GLEW include cost.
- **No bounds checking on indices.** If an index in `indices` is ≥ `vertex_count_`, the GPU reads out-of-bounds. GL doesn't care; you get garbage or a driver crash.
- **Topology is always `GL_TRIANGLES`.** No lines, points, strips, fans. Those would need a parameter.

---

## See also

- [04 — Graphics API](./engine-04-graphics-api.md) — where `create_vertex_buffer` and `create_index_buffer` live.
- [08 — Walkthrough](./engine-08-walkthrough.md) — a concrete `VertexLayout` + `Mesh` built from scratch.
- [vao-vbo-shaders.md](./vao-vbo-shaders.md) — background on VAO/VBO/EBO.
- [meshes-and-models.md](./meshes-and-models.md) — where meshes come from (file formats, loaders).
