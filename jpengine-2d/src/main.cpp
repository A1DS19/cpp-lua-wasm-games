#include "ecs/component.hpp"
#include "ecs/entity.hpp"
#include "ecs/registry.hpp"
#include "rendering/batch-renderer.hpp"
#include "rendering/camera.hpp"
#include "rendering/default-shaders.hpp"
#include "rendering/font.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"
#include "rendering/vertex.hpp"
#include "scripting/glm_bindings.hpp"
#include "utils/asset-loader.hpp"

#include <SDL_pixels.h>
#include <SDL_surface.h>
#include <cstddef>
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <sol/state.hpp>
#include <sol/types.hpp>
#include <string_view>
#include <sys/ucontext.h>
#include <vector>
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
#include <sol/sol.hpp>
#include <string>
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

using namespace jpengine;

SDL_Window* p_window{nullptr};
SDL_GLContext gl_context{};

GLuint vao{0};
GLuint vbo{0};
GLuint ebo{0};

std::shared_ptr<Shader> shader{nullptr};
std::shared_ptr<Texture> texture{nullptr};
std::unique_ptr<Camera> camera{nullptr};
std::unique_ptr<BatchRenderer> pbatch_renderer{nullptr};
std::unique_ptr<Registry> pregistry{nullptr};
std::shared_ptr<Font> pfont{nullptr};

sol::protected_function script_update;
std::unique_ptr<Registry> registry = nullptr;

void register_meta_components() {
    Entity::register_meta_component<Identification>();
    Entity::register_meta_component<TransformComponent>();
    Entity::register_meta_component<SpriteComponent>();
    Entity::register_meta_component<BoxColider>();
    Entity::register_meta_component<CircleCollider>();
    Entity::register_meta_component<AnimationComponent>();
    Entity::register_meta_component<TransformComponent>();
    Entity::register_meta_component<RigidBodyComponent>();

    Registry::register_meta_component<Identification>();
    Registry::register_meta_component<TransformComponent>();
    Registry::register_meta_component<SpriteComponent>();
    Registry::register_meta_component<BoxColider>();
    Registry::register_meta_component<CircleCollider>();
    Registry::register_meta_component<AnimationComponent>();
    Registry::register_meta_component<TransformComponent>();
    Registry::register_meta_component<RigidBodyComponent>();
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

    std::vector<Vertex> vertices;
    vertices.resize(6);
    vertices[0] = Vertex{.position_ = glm::vec2{50.F, 0.F}, .uvs_ = UV{.u_ = 1.F, .v_ = 0.F}};
    vertices[1] = Vertex{.position_ = glm::vec2{50.F, 50.F}, .uvs_ = UV{.u_ = 1.F, .v_ = 1.F}};
    vertices[2] = Vertex{.position_ = glm::vec2{0.F, 0.F}, .uvs_ = UV{.u_ = 0.F, .v_ = 0.F}};
    vertices[3] = Vertex{.position_ = glm::vec2{50.F, 50.F}, .uvs_ = UV{.u_ = 1.F, .v_ = 1.F}};
    vertices[4] = Vertex{.position_ = glm::vec2{0.F, 50.F}, .uvs_ = UV{.u_ = 0.F, .v_ = 1.F}};
    vertices[5] = Vertex{.position_ = glm::vec2{0.F, 0.F}, .uvs_ = UV{.u_ = 0.F, .v_ = 0.F}};

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, position_));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvs_));
    glEnableVertexAttribArray(1);

    pfont = utils::AssetLoader::load_font("assets/fonts/pixel.ttf");
    if (!pfont) {
        std::cerr << "failed to load pixel font" << "\n";
        return false;
    }

    pbatch_renderer = std::make_unique<BatchRenderer>();

    texture = jpengine::utils::AssetLoader::load_texture("assets/textures/character.png", true);
    if (texture == nullptr) {
        std::cerr << "failed to load texture [character.png]";
        return false;
    }

    camera = std::make_unique<Camera>(800, 600);
    std::cout << "sdl/opengl initialization success\n";

    auto lua = std::make_shared<sol::state>();
    registry = std::make_unique<Registry>();
    camera->update();
    lua->open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::math);

    register_meta_components();

    Camera::create_lua_bind(*lua, *camera);
    Vertex::create_lua_bind(*lua);
    GlmBinder::create_lua_bind(*lua);
    ComponentBinder::create_lua_bind(*lua);
    Registry::create_lua_bind(*lua, *registry);
    Entity::create_lua_bind(*lua, *registry);

    Entity ent3{*registry};
    ent3.add_component<TransformComponent>(TransformComponent{
        .position_ = glm::vec2{200.F}, .scale_ = glm::vec2{4.F}, .rotation_ = 45.F});

    auto lua_ctx = registry->add_to_context<std::shared_ptr<sol::state>>(std::move(lua));
    auto result = lua_ctx->do_file("assets/scripts/main.lua");
    if (result.valid()) {
        script_update = result.get<sol::protected_function>(0);
    } else {
        sol::error err = result;
        std::cerr << "Failed to load script: " << err.what() << "\n";
    }

    return true;
}

void render_sprites() {
    auto view = registry->get_registry().view<TransformComponent, SpriteComponent>();
    pbatch_renderer->begin();

    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        const auto& sprite = view.get<SpriteComponent>(entity);

        if (sprite.hidden_ || sprite.string_.empty())
            continue;

        glm::vec4 pos{transform.position_.x, transform.position_.y,
                      sprite.width_ * transform.scale_.x, sprite.height_ * transform.scale_.y};

        glm::vec4 uvs{sprite.uvs_.u_, sprite.uvs_.v_, sprite.uvs_.uv_widht_,
                      sprite.uvs_.uv_height_};

        pbatch_renderer->add_sprite(pos, uvs, sprite.layer_, texture->get_id(), sprite.color_);
    }

    pbatch_renderer->end();
    pbatch_renderer->render();
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

    glBindVertexArray(vao);

    auto camera_matrix = camera->get_camera_matrix();
    shader->set_uniform_mat4("u_projection", camera_matrix);

    render_sprites();

    shader->disable();
    SDL_GL_SwapWindow(p_window);

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
