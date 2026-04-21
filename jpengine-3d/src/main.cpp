#include "core/macros.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

int main() {
#ifdef __linux__
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

    if (!core::macros::convert_to_bool(glfwInit())) {
        return EXIT_FAILURE;
    }

    // Window / context hints must be set BEFORE glfwCreateWindow.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* pwindow = glfwCreateWindow(1280, 720, "jpengine-3d", nullptr, nullptr);
    if (!pwindow) {
        std::cerr << "error creating window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(pwindow);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "error initializing GLEW\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::vector<float> vertices{
        //  x      y     z     r     g     b
        0.0F,  0.5F,  0.0F, 1.0F, 0.0F, 0.0F, // vertex 0 — top          (red)
        -0.5F, -0.5F, 0.0F, 0.0F, 1.0F, 0.0F, // vertex 1 — bottom-left  (green)
        0.5F,  -0.5F, 0.0F, 0.0F, 0.0F, 1.0F, // vertex 2 — bottom-right (blue)
    };

    constexpr GLsizei stride = 6 * sizeof(float);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // attribute 0 — position (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // attribute 1 — color (r, g, b), offset past the 3 position floats
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    const std::string vertex_shader_src = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        out vec3 v_color;

        void main() {
            gl_Position = vec4(position, 1.0);
            v_color = color;
        }
    )";

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_shader_cstr = vertex_shader_src.c_str();
    glShaderSource(vertex_shader, 1, &vertex_shader_cstr, nullptr);
    glCompileShader(vertex_shader);

    GLint success_vertex_shader = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success_vertex_shader);
    if (!core::macros::convert_to_bool(success_vertex_shader)) {
        char info_log[512];
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        std::cerr << "err vertex shader compilation failed: " << info_log << "\n";
    }

    const std::string fragment_shader_src = R"(
        #version 330 core
        out vec4 frag_color;
        in vec3 v_color;

        void main() {
            frag_color = vec4(v_color, 1.0);
        }
    )";

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_shader_cstr = fragment_shader_src.c_str();
    glShaderSource(fragment_shader, 1, &fragment_shader_cstr, nullptr);
    glCompileShader(fragment_shader);

    GLint success_fragment_shader = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success_fragment_shader);
    if (!core::macros::convert_to_bool(success_fragment_shader)) {
        char info_log[512];
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        std::cerr << "err fragment shader compilation failed: " << info_log << "\n";
    }

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    GLint success_program = 0;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success_program);
    if (!core::macros::convert_to_bool(success_program)) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
        std::cerr << "err shader program linking failed: " << info_log << "\n";
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    while (!core::macros::convert_to_bool(glfwWindowShouldClose(pwindow))) {
        glClearColor(1.0F, 1.0F, 1.0F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(pwindow);
        glfwPollEvents();
    }

    glDeleteProgram(shader_program);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return EXIT_SUCCESS;
}
