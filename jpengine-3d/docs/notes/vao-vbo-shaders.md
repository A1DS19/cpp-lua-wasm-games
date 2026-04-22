# VAO, VBO, EBO, and the Minimal Draw Pipeline

## What problem are these solving?

The GPU is a separate processor with its own memory. It can't read your `std::vector<float>` directly — you have to **upload** that data to a buffer on the GPU and tell the GPU **how to interpret** it. Three objects cover the basics:

| Object | Holds              | Role                                          |
|--------|--------------------|-----------------------------------------------|
| **VBO** | vertex data       | the ingredients                                |
| **EBO** | indices           | the order to eat them in                       |
| **VAO** | bindings + layout | the recipe card that points at VBO and EBO     |

Plus a **shader program** — the GPU code that runs on each vertex/fragment.

---

## VBO — Vertex Buffer Object

A **VBO** is a **blob of bytes on the GPU**. You hand it an `std::vector<float>` (or any raw bytes) and that data now lives in video memory.

```cpp
GLuint vbo = 0;
glGenBuffers(1, &vbo);                 // ask GL for a buffer handle
glBindBuffer(GL_ARRAY_BUFFER, vbo);    // "the next ARRAY_BUFFER op acts on vbo"
glBufferData(GL_ARRAY_BUFFER,          // upload data
             vertices.size() * sizeof(float),
             vertices.data(),
             GL_STATIC_DRAW);          // usage hint
```

Key facts:

- A VBO has **no structure of its own** — it doesn't know those 24 floats are 4 vertices with 6 components each. That interpretation comes from the VAO.
- `GL_STATIC_DRAW` is a usage hint: "upload once, draw many times." Others:
  - `GL_DYNAMIC_DRAW` — re-upload often.
  - `GL_STREAM_DRAW` — re-upload every frame.
- The target `GL_ARRAY_BUFFER` is for vertex data. Indices use `GL_ELEMENT_ARRAY_BUFFER`.

### Typical layout — interleaved attributes

For a vertex with position + color, you usually pack them together:

```cpp
std::vector<float> vertices{
//   x      y     z    r    g    b
    0.5F,  0.5F, 0.0F, 1.0F, 0.0F, 0.0F,
   -0.5F,  0.5F, 0.0F, 0.0F, 1.0F, 0.0F,
   -0.5F, -0.5F, 0.0F, 0.0F, 0.0F, 1.0F,
    0.5F, -0.5F, 0.0F, 1.0F, 1.0F, 0.0F,
};
```

Each vertex is **6 floats = 24 bytes** — that's the **stride**.

---

## EBO — Element Buffer Object

Same kind of object as a VBO — a **blob of bytes on the GPU** — but it holds **indices** (integers) that index into your VBO's vertex list. Also called an **IBO** (Index Buffer Object).

### Why you want one

A quad has 4 unique corners but 2 triangles = 6 drawn vertices. Without an EBO you'd duplicate 2 vertices:

```
6 vertex entries — wasteful: { v0, v1, v2,   v0, v2, v3 }
```

With an EBO:

```
4 unique vertices + 6 indices — each vertex stored once
vertices = { v0, v1, v2, v3 };
indices  = { 0, 1, 2,   0, 2, 3 };
```

For a cube that's 8 unique vertices instead of 36. For a character model, 20k vs 100k. Huge memory and GPU-cache win.

### Creating and uploading

```cpp
std::vector<unsigned int> indices = { 0, 1, 2,  0, 2, 3 };

GLuint ebo = 0;
glGenBuffers(1, &ebo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER,
             indices.size() * sizeof(unsigned int),
             indices.data(),
             GL_STATIC_DRAW);
```

### Drawing with an EBO

Swap `glDrawArrays` for `glDrawElements`:

```cpp
glDrawElements(GL_TRIANGLES,
               static_cast<GLsizei>(indices.size()),   // 6
               GL_UNSIGNED_INT,
               nullptr);
```

---

## VAO — Vertex Array Object

A **VAO** is a **recipe** for how to draw. It remembers:

1. How each vertex attribute is laid out (offset, stride, type, component count).
2. Which attributes are enabled.
3. Which **VBO** each attribute reads from (captured at the moment you call `glVertexAttribPointer`).
4. Which **EBO** provides the indices.

Without a VAO, you'd re-specify all attribute state before every draw call. With a VAO you bind it once and issue a draw.

```cpp
GLuint vao = 0;
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);            // start recording

glBindBuffer(GL_ARRAY_BUFFER, vbo);
glVertexAttribPointer(
    0,                             // layout(location = 0) in the shader
    3,                             // 3 components per vertex (x, y, z)
    GL_FLOAT,                      // component type
    GL_FALSE,                      // no normalization
    6 * sizeof(float),             // stride: bytes between vertices
    (void*)0                       // offset to first component
);
glEnableVertexAttribArray(0);

glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                      6 * sizeof(float),
                      (void*)(3 * sizeof(float)));  // offset past xyz
glEnableVertexAttribArray(1);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);   // recorded into the VAO

glBindVertexArray(0);              // stop recording
```

---

## Binding order — the single most-missed rule

The VAO captures state depending on which target you bind to. Two distinct behaviors:

### `GL_ARRAY_BUFFER` (VBO) — captured **via `glVertexAttribPointer`**

The VAO does **not** directly remember the current `GL_ARRAY_BUFFER` binding. Instead, `glVertexAttribPointer` captures whichever VBO is bound **at that instant** and links it to the attribute. You can bind a different VBO afterwards; the attribute still points at the original.

### `GL_ELEMENT_ARRAY_BUFFER` (EBO) — captured **directly**

The VAO records whatever EBO is bound to `GL_ELEMENT_ARRAY_BUFFER` while the VAO is bound. There's no `glElementArrayPointer`-like function; the binding *itself* is the state.

### The practical rule

> **Bind the VAO first. Do everything else inside.**

```cpp
glBindVertexArray(vao);                       // 1. VAO first

glBindBuffer(GL_ARRAY_BUFFER, vbo);           // 2. VBO
glVertexAttribPointer(0, ...);                //    ← captures VBO now
glEnableVertexAttribArray(0);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);   // 3. EBO recorded by VAO
glBufferData(GL_ELEMENT_ARRAY_BUFFER, ...);   //    (upload while VAO bound)

glBindVertexArray(0);                         // 4. stop recording
```

### Unbind order matters too

`glBindVertexArray(0)` unbinds the VAO and **freezes its state**. Any `GL_ELEMENT_ARRAY_BUFFER` unbind *after* that is harmless. But if you unbind the EBO **before** the VAO, you've just overwritten the VAO's EBO slot with 0.

**Safe:**
```cpp
glBindVertexArray(0);                     // unbind VAO first
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // then EBO (no effect on VAO)
```

**Broken (EBO gets unrecorded):**
```cpp
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // VAO still bound — EBO slot zeroed!
glBindVertexArray(0);
```

(`GL_ARRAY_BUFFER` is safe to unbind early — the VAO doesn't track it directly.)

### Golden rule

**A VAO records what's bound — it doesn't own the buffers.** If you delete a VBO or EBO while a VAO still references it, the VAO breaks. Keep buffers alive for the VAO's lifetime.

---

## Shader program

A **program** is a linked bundle of at least a vertex shader and a fragment shader.

```cpp
// 1. Compile each shader
GLuint vs = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(vs, 1, &src_ptr, nullptr);
glCompileShader(vs);
// ...check GL_COMPILE_STATUS, read log on failure...

// 2. Link them into a program
GLuint program = glCreateProgram();
glAttachShader(program, vs);
glAttachShader(program, fs);
glLinkProgram(program);
// ...check GL_LINK_STATUS, read log on failure...

// 3. Delete the shaders — the program keeps its own copy
glDeleteShader(vs);
glDeleteShader(fs);
```

Always check compile status (`GL_COMPILE_STATUS`) and link status (`GL_LINK_STATUS`) and read the info log — a GLSL typo gives a black screen and nothing else.

---

## The minimal draw loop

Once VBO, EBO, VAO, and program exist, each frame is tiny:

```cpp
glClearColor(1, 1, 1, 1);
glClear(GL_COLOR_BUFFER_BIT);

glUseProgram(program);
glBindVertexArray(vao);
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);   // with EBO
// or: glDrawArrays(GL_TRIANGLES, 0, 3);                     // without EBO

glfwSwapBuffers(window);
glfwPollEvents();
```

---

## Correct init order (wrong order = segfault)

```
1. glfwInit()
2. glfwWindowHint(...)                 ← set version + profile BEFORE createWindow
3. glfwCreateWindow(...)               ← returns null if hints can't be satisfied
4. check window != nullptr
5. glfwMakeContextCurrent(window)
6. glewExperimental = GL_TRUE
7. glewInit()                          ← only now can you call gl*() functions
8. create VBO, EBO, VAO, compile shaders, link program
9. render loop
10. cleanup + glfwTerminate()
```

**Calling any `gl*` function before step 7 segfaults** — there's no GL context yet, and GLEW hasn't resolved the function pointers, so `glGenBuffers` is a null-pointer dereference.

### macOS gotcha

macOS only ships OpenGL up to 4.1, and requires the forward-compat flag for core profile:

```cpp
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
```

Without it, `glfwCreateWindow` returns `nullptr` on macOS → `glfwMakeContextCurrent(nullptr)` → crash.

---

## Mental model

| Object     | Owns           | Stores                                       | Lifetime rule                                       |
|------------|----------------|----------------------------------------------|------------------------------------------------------|
| **VBO**    | raw bytes      | vertex data                                  | needs to outlive any VAO that references it          |
| **EBO**    | raw bytes      | indices                                      | needs to outlive any VAO that references it          |
| **VAO**    | state          | attribute layout + which VBO + which EBO     | independent                                           |
| **Shader program** | GPU code | compiled+linked vertex & fragment shaders  | independent                                           |

To draw something: bind a VAO (the recipe), bind a program (the code), call a draw function. That's it.
