# VAO, VBO, and the Minimal Triangle Pipeline

## What problem are these solving?

The GPU is a separate processor with its own memory. It can't read your `std::vector<float>` directly — you have to **upload** that data to a buffer on the GPU and tell the GPU **how to interpret** it. VBO and VAO are the two objects that do this.

## VBO — Vertex Buffer Object

A **VBO** is just a **blob of bytes on the GPU**. You hand it an `std::vector<float>` (or any raw bytes) and that data now lives in video memory.

```cpp
GLuint vbo = 0;
glGenBuffers(1, &vbo);                 // ask GL for a buffer handle
glBindBuffer(GL_ARRAY_BUFFER, vbo);    // "the next ARRAY_BUFFER op acts on vbo"
glBufferData(GL_ARRAY_BUFFER,          // upload data
             vertices.size() * sizeof(float),
             vertices.data(),
             GL_STATIC_DRAW);          // usage hint: write once, draw many times
```

Key facts:
- A VBO has **no structure of its own** — it doesn't know those 9 floats are 3 vertices.
- `GL_STATIC_DRAW` is a hint: "I'll upload once and draw many times." Others: `GL_DYNAMIC_DRAW` (re-upload often), `GL_STREAM_DRAW` (upload every frame).
- The buffer target `GL_ARRAY_BUFFER` is for vertex data. Index buffers use `GL_ELEMENT_ARRAY_BUFFER` (those are called **EBOs**).

## VAO — Vertex Array Object

A **VAO** is a **recipe** for how to read a VBO. It stores:

1. Which VBO to pull vertex data from.
2. How the bytes are laid out — offsets, stride, types, component counts.
3. Which attribute locations in the shader they map to.
4. Which attributes are enabled.
5. (If bound) which EBO provides the indices.

```cpp
GLuint vao = 0;
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);                 // "record all following state into vao"

glBindBuffer(GL_ARRAY_BUFFER, vbo);     // VAO remembers this binding
glVertexAttribPointer(
    0,                   // layout(location = 0) in the shader
    3,                   // 3 components per vertex (x, y, z)
    GL_FLOAT,            // component type
    GL_FALSE,            // no normalization
    3 * sizeof(float),   // stride between vertices
    (void*)0             // offset to first component in the VBO
);
glEnableVertexAttribArray(0);           // turn this attribute on

glBindVertexArray(0);                   // stop recording
```

Without a VAO you'd re-specify all attribute state before every draw call. With a VAO, you bind it once and call `glDrawArrays` — the GPU already knows the layout.

### The golden rule

**A VAO records what's bound — it doesn't own the VBO.** If you delete the VBO, the VAO breaks. Keep the VBO alive for the VAO's lifetime.

## EBO — Element Buffer Object (bonus)

Same as a VBO but for **indices**. Lets you reuse shared vertices between triangles.

```cpp
unsigned indices[] = { 0, 1, 2,  0, 2, 3 };   // two triangles sharing edge 0-2

GLuint ebo = 0;
glGenBuffers(1, &ebo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);   // MUST be bound while VAO is bound
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

// draw:
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
```

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

Always check compile status (`GL_COMPILE_STATUS`) and link status (`GL_LINK_STATUS`) and read the info log — a typo in GLSL gives a black screen and nothing else.

## The minimal draw loop

Once VBO, VAO, and program exist, each frame is tiny:

```cpp
glClearColor(1, 1, 1, 1);
glClear(GL_COLOR_BUFFER_BIT);

glUseProgram(program);
glBindVertexArray(vao);
glDrawArrays(GL_TRIANGLES, 0, 3);    // "draw 3 vertices as 1 triangle"

glfwSwapBuffers(window);
glfwPollEvents();
```

## Correct init order (important — wrong order = segfault)

```
1. glfwInit()
2. glfwWindowHint(...)                 ← set version + profile BEFORE createWindow
3. glfwCreateWindow(...)               ← returns null if hints can't be satisfied
4. check window != nullptr
5. glfwMakeContextCurrent(window)
6. glewExperimental = GL_TRUE
7. glewInit()                          ← only now can you call gl*() functions
8. create VBO, VAO, compile shaders, link program
9. render loop
10. cleanup + glfwTerminate()
```

**Calling any `gl*` function before step 7 segfaults** — there's no GL context yet, and GLEW hasn't resolved the function pointers, so `glGenBuffers` is a null pointer dereference.

### macOS gotcha

macOS only ships OpenGL up to 4.1, and requires the forward-compat flag for core profile:

```cpp
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
```

Without it, `glfwCreateWindow` returns `nullptr` on macOS → `glfwMakeContextCurrent(nullptr)` → crash.

## Mental model

| Object   | Owns           | Stores                                    | Lifetime  |
|----------|----------------|-------------------------------------------|-----------|
| **VBO**  | raw bytes      | the vertex data                           | needs to outlive any VAO that references it |
| **VAO**  | state          | attribute layout + which VBO/EBO to use   | independent |
| **EBO**  | raw bytes      | the indices                               | needs to outlive the VAO |
| **Shader program** | GPU code | compiled+linked vertex & fragment shaders | independent |

To draw something, you bind a VAO (the recipe) and a program (the code), and call a draw function. That's it.
