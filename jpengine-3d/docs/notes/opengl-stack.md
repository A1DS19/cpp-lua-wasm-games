# OpenGL Stack — GLFW, GLEW, OpenGL

## What each library does

| Library    | Role |
|------------|------|
| **OpenGL** | The graphics API itself — the ~3000 functions that tell the GPU to draw. |
| **GLFW**   | Windowing + input toolkit: creates the OS window, the OpenGL context, and forwards keyboard/mouse/gamepad/monitor events. Cross-platform (Win/macOS/Linux). |
| **GLEW**   | Function-pointer loader. Resolves OpenGL functions at runtime so you can call them like normal C functions. |

They're **complementary**: GLFW gives you a window + GL context, GLEW gives you callable GL functions, OpenGL draws.

## Why GLEW (or any loader) exists

On Windows/Linux, the OS-provided GL library (`opengl32.dll`, `libGL.so`) only exposes **OpenGL 1.1** symbols. Everything added after 1998 — shaders, VBOs, VAOs, framebuffers, compute — lives inside the graphics driver and must be resolved at runtime:

```cpp
auto glCreateShader = (PFNGLCREATESHADERPROC)
    wglGetProcAddress("glCreateShader");   // Windows
    // or glXGetProcAddress (Linux), eglGetProcAddress (GLES)
```

Do that for ~3000 functions and you've built GLEW. GLEW declares the pointers, loads them with one `glewInit()` call, and lets you call `glCreateShader(...)` directly.

### macOS caveat

macOS historically shipped a full static GL library — everything up to 4.1 was directly linkable via `-framework OpenGL`, no loader needed. But Apple **deprecated OpenGL in 10.14** and capped it at 4.1 forever. Use GLEW anyway for cross-platform code; just know you're stuck at 4.1 on macOS.

## Modern alternatives to GLEW

| Loader         | Notes |
|----------------|-------|
| **GLAD**       | Web generator, you pick version/profile/extensions. Smaller, compile-time-known surface. **Most new projects use this.** |
| **glbinding**  | C++17, type-safe enums, heavier. |
| **SDL built-in** | `SDL_GL_GetProcAddress` if you already use SDL. |

## Minimal usage pattern

```cpp
#include <GL/glew.h>     // must come BEFORE glfw3.h
#include <GLFW/glfw3.h>

int main() {
    glfwInit();
    GLFWwindow* win = glfwCreateWindow(800, 600, "jpengine", nullptr, nullptr);
    glfwMakeContextCurrent(win);

    glewExperimental = GL_TRUE;   // needed for core profile
    glewInit();                   // must be AFTER makeContextCurrent

    while (!glfwWindowShouldClose(win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        // draw stuff
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
}
```

**Order matters**: GL context → `glewInit()` → any GL call.
