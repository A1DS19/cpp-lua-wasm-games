#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#else
#  include <GLES3/gl3.h>
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL_events.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sol/sol.hpp>
#include <string>
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

SDL_Window* p_window{nullptr};
SDL_GLContext gl_context{};

const char* vertex_shader_ = R"(#version 300 es
in vec3 a_pos;
void main(){
  gl_Position = vec4(a_pos, 1.0);
}
)";

const char* frag_shader_ = R"(#version 300 es
precision mediump float;
out vec4 fragColor;
void main(){
  fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

GLuint vao{0};
GLuint vbo{0};
GLuint ebo{0};
GLuint shader_program{0};

GLuint load_shader_from_memory(const char* vertex_shader, const char* frag_shader);

bool init_sdl() {
    std::cout << "initializing sdl\n";
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "sdl initialization failed: " << SDL_GetError() << "\n";
        return EXIT_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    p_window = SDL_CreateWindow("sdl and emscripten test", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);

    if (p_window == nullptr) {
        std::cerr << "failed to create sdl window: " << SDL_GetError() << "\n";
    }

    gl_context = SDL_GL_CreateContext(p_window);
    // enable vsync
    SDL_GL_SetSwapInterval(1);
    shader_program = load_shader_from_memory(vertex_shader_, frag_shader_);

    if (shader_program == 0) {
        std::cerr << "failed to load shaders\n";
        return false;
    }

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  // v0 bottom-left
         0.5f, -0.5f, 0.0f,  // v1 bottom-right
         0.5f,  0.5f, 0.0f,  // v2 top-right
        -0.5f,  0.5f, 0.0f,  // v3 top-left
    };
    unsigned int indices[] = {
        0, 1, 2,  // triangle 1
        0, 2, 3,  // triangle 2
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::cout << "sdl/opengl initialization success\n";

    return true;
}

GLuint load_shader_from_memory(const char* vertex_shader, const char* frag_shader) {
    const GLuint program = glCreateProgram();

    // vertex shader
    const GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vertex_shader, nullptr);
    glCompileShader(vert_shader);
    GLint status;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint max_length;
        glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(static_cast<size_t>(max_length), ' ');
        glGetShaderInfoLog(vert_shader, max_length, &max_length, error_log.data());
        std::cerr << "GLSL compile failed: " << std::string{error_log} << std::endl;
        glDeleteShader(vert_shader);
        glDeleteProgram(program);
        return 0;
    }

    // fragment shader
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_shader, nullptr);
    glCompileShader(fragment_shader);
    GLint frag_status;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &frag_status);
    if (frag_status != GL_TRUE) {
        GLint max_length;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(static_cast<size_t>(max_length), ' ');
        glGetShaderInfoLog(fragment_shader, max_length, &max_length, error_log.data());
        std::cerr << "GLSL compile failed: " << std::string{error_log} << std::endl;
        glDeleteShader(vert_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);
        return 0;
    }

    if (vert_shader == 0 || fragment_shader == 0) {
        return 0;
    }

    glAttachShader(program, vert_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        GLint max_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(static_cast<size_t>(max_length), ' ');
        glGetProgramInfoLog(program, max_length, &max_length, error_log.data());
        std::cerr << "GLSL link failed: " << std::string{error_log} << std::endl;
        glDeleteShader(vert_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }

    if (program == 0) {
        std::cerr << "failed to load shader from memory, program invalid\n";
        return 0;
    }

    return program;
}

void cleanup() {
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(p_window);
    SDL_Quit();
}

void game_loop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
        }
    }

    SDL_GL_MakeCurrent(p_window, gl_context);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int w, h;
    SDL_GetWindowSize(p_window, &w, &h);
    glViewport(0.0f, 0.0f, w, h);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    SDL_GL_SwapWindow(p_window);
}

auto main() -> int {
    std::cout << "starting game...\n";
    if (!init_sdl()) {
        return EXIT_FAILURE;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(game_loop, 0, 1);
#else
    while (true) {
        game_loop();
    }
#endif
    cleanup();

    return EXIT_SUCCESS;
}
