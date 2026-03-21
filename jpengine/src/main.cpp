#include "rendering/camera.hpp"
#include "rendering/default-shaders.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"
#include "utils/asset-loader.hpp"

#include <SDL_pixels.h>
#include <SDL_surface.h>
#include <cstddef>
#include <memory>
#include <sol/state.hpp>
#include <sol/types.hpp>
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

#include <chrono>
#include <filesystem>

using namespace jpengine;

SDL_Window* p_window{nullptr};
SDL_GLContext gl_context{};

GLuint vao{0};
GLuint vbo{0};
GLuint ebo{0};

std::shared_ptr<Shader> shader{nullptr};
std::shared_ptr<Texture> texture{nullptr};
std::unique_ptr<Camera> camera{nullptr};

sol::state lua;
sol::protected_function script_update;
std::filesystem::file_time_type last_script_write_time;
const std::string script_path = "assets/scripts/main.lua";

void reload_script() {
    try {
        if (!std::filesystem::exists(script_path)) {
            return;
        }

        auto current_time = std::filesystem::last_write_time(script_path);
        if (current_time > last_script_write_time) {
            std::cout << "Reloading script: " << script_path << "\n";
            auto result = lua.do_file(script_path);
            if (result.valid()) {
                script_update = result.get<sol::protected_function>(0);
                last_script_write_time = current_time;
            } else {
                sol::error err = result;
                std::cerr << "Failed to reload script: " << err.what() << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error checking for script updates: " << e.what() << "\n";
    }
}

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
    shader = jpengine::utils::AssetLoader::load_shader_from_memory(
        DefaultShaders::basic_shader_vert, DefaultShaders::basic_shader_frag);

    if (shader == nullptr) {
        std::cerr << "failed to load shaders\n";
        return false;
    }

    // A 200x200 quad centered at (400, 300) — screen center for 800x600
    auto vertices = std::array<float, 16>{
        0.0f,   0.0f,   0.0f, 0.0f, // v0 top-left
        200.0f, 0.0f,   1.0f, 0.0f, // v1 top-right
        200.0f, 200.0f, 1.0f, 1.0f, // v2 bottom-right
        0.0f,   200.0f, 0.0f, 1.0f, // v3 bottom-left
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

    camera = std::make_unique<Camera>(800, 600);
    std::cout << "sdl/opengl initialization success\n";

    camera->update();
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::math);
    Camera::create_lua_bind(lua, *camera);

    // Initial load
    reload_script();

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

    auto camera_matrix = camera->get_camera_matrix();
    shader->set_uniform_mat4("u_projection", camera_matrix);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);

    texture->disable();
    shader->disable();
    SDL_GL_SwapWindow(p_window);

    // Dynamic Hot Reload check
#ifndef __EMSCRIPTEN__
    reload_script();
#endif

    if (script_update.valid()) {
        auto result = script_update();
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Lua execution error: " << err.what() << "\n";
        }
    }

    camera->update();
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
