#include "rendering/shader.hpp"
#include "rendering/texture.hpp"
#include "utils/asset-loader.hpp"

#include <SDL_pixels.h>
#include <SDL_surface.h>
#include <cstddef>
#include <memory>
#include <string_view>
#ifdef __APPLE__
    #include <OpenGL/gl3.h>
#else
    #include <GLES3/gl3.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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

using namespace jpengine;

SDL_Window* p_window{nullptr};
SDL_GLContext gl_context{};

const char* vertex_shader_ = R"(#version 300 es
in vec2 a_pos;
in vec2 a_uvs;
out vec2 frag_uvs;

void main(){
  gl_Position = vec4(a_pos, 0.0f, 1.0f);
  frag_uvs = a_uvs;
}
)";

const char* frag_shader_ = R"(#version 300 es
precision mediump float;
in vec2 frag_uvs;
out vec4 fragColor;
uniform sampler2D u_texture;

void main(){
  fragColor = texture(u_texture, frag_uvs);
}
)";

GLuint vao{0};
GLuint vbo{0};
GLuint ebo{0};

std::shared_ptr<Shader> shader{nullptr};
std::shared_ptr<Texture> texture{nullptr};

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
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    shader = jpengine::utils::AssetLoader::load_shader_from_memory(vertex_shader_, frag_shader_);

    if (shader == nullptr) {
        std::cerr << "failed to load shaders\n";
        return false;
    }

    auto vertices = std::array<float, 16>{
        -0.5f, -0.5f, 0.0f, 1.0f, // v0 bottom-left
        0.5f,  -0.5f, 1.0f, 1.0f, // v1 bottom-right
        0.5f,  0.5f,  1.0f, 0.0f, // v2 top-right
        -0.5f, 0.5f,  0.0f, 0.0f, // v3 top-left
    };

    auto indices = std::array<uint8_t, 6>{
        0, 1, 2, // triangle 1
        0, 2, 3, // triangle 2
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    texture = jpengine::utils::AssetLoader::load_texture("assets/textures/character.png", true);
    if (texture == nullptr) {
        std::cerr << "failed to load texture [character.png]";
        return false;
    }

    std::cout << "sdl/opengl initialization success\n";
    return true;
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

    shader->enable();
    texture->enable();

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);

    texture->disable();
    shader->disable();
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
