#include "ecs/component.hpp"
#include "ecs/entity.hpp"
#include "ecs/registry.hpp"
#include "inputs/gamepad.hpp"
#include "inputs/keyboard.hpp"
#include "inputs/mouse.hpp"
#include "rendering/batch-renderer.hpp"
#include "rendering/camera.hpp"
#include "rendering/default-shaders.hpp"
#include "rendering/font.hpp"
#include "rendering/shader.hpp"
#include "rendering/text-batch-renderer.hpp"
#include "rendering/texture.hpp"
#include "rendering/vertex.hpp"
#include "scripting/glm_bindings.hpp"
#include "scripting/script_bindings.hpp"
#include "sounds/music_player.hpp"
#include "sounds/sound_player.hpp"
#include "utils/asset-manager.hpp"

#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <sol/forward.hpp>
#include <sol/optional_implementation.hpp>
#include <sol/state.hpp>
#include <sol/types.hpp>
#include <string_view>
#include <sys/ucontext.h>
#ifdef __APPLE__
    #include <OpenGL/gl3.h>
#else
    #include <GLES3/gl3.h>
#endif

#include "game.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_video.h>
#include <SDL_events.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <iostream>
#include <sol/sol.hpp>
#include <string>
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

using namespace jpengine;

void InputContext::update() {
    if (pkeyboard_) {
        pkeyboard_->update();
    }

    if (pmouse_) {
        pmouse_->update();
    }

    if (pgamepad_) {
        pgamepad_->update();
    }
}

Game::Game()
    : pwindow_{nullptr}, pglcontext_{}, pregistry_{nullptr}, main_script_{}, runnning_{false},
      event_{} {}

Game::~Game() = default;

void Game::run() {
    process_events();
    update();
    render();
}

bool Game::initialize() {
    if (!init_sdl()) {
        std::cerr << "failed to initialize sdl\n";
        return false;
    }

    if (!initialize_registry()) {
        std::cerr << "failed to initialize registry\n";
        return false;
    }

    if (!load_shaders()) {
        std::cerr << "failed to load shaders succesfully\n";
    }

    register_meta_components();
    register_lua_bindings();

    if (!load_main_script()) {
        std::cerr << "failed to load main lua script\n";
        return false;
    }

    return true;
}

bool Game::initialize_registry() {
    pregistry_ = std::make_unique<Registry>();
    auto plua_state = pregistry_->add_to_context<SolStatePtr>(std::make_shared<sol::state>());
    plua_state->open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::math);
    pregistry_->add_to_context<CameraPtr>(std::make_shared<Camera>(800, 600));

    auto pinput_context = std::make_shared<InputContext>();
    pinput_context->pkeyboard_ = std::make_shared<Keyboard>();
    pinput_context->pgamepad_ = std::make_shared<Gamepad>();
    pinput_context->pmouse_ = std::make_shared<Mouse>();

    auto paudio_context = std::make_shared<AudioContext>();
    paudio_context->pmusic_player_ = std::make_shared<MusicPlayer>();
    paudio_context->psound_player_ = std::make_shared<SoundPlayer>();
    pregistry_->add_to_context<AudioCtxPtr>(std::move(paudio_context));

    pregistry_->add_to_context<InputCtxPtr>(std::move(pinput_context));
    pregistry_->add_to_context<BatchRendererPtr>(std::make_shared<BatchRenderer>());
    pregistry_->add_to_context<TextBatchRendererPtr>(std::make_shared<TextBatchRenderer>());
    pregistry_->add_to_context<AssetManagerPtr>(std::make_shared<AssetManager>());

    return true;
}

bool Game::load_main_script() {
    auto& plua_state = pregistry_->get_context<SolStatePtr>();
    auto result =
        plua_state->safe_script_file("assets/scripts/main.lua", sol::script_pass_on_error);

    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "failed to load main lua script: " << err.what() << "\n";
        return false;
    }

    sol::optional<sol::table> opt_main_table = (*plua_state)["main"];
    if (!opt_main_table) {
        std::cerr << "failed to load main script, main table does not exist\n";
        return false;
    }

    sol::optional<sol::protected_function> opt_update_func = (*plua_state)["main"]["update"];
    if (!opt_update_func) {
        std::cerr << "failed to load update function, update function does not exists\n";
        return false;
    }

    main_script_.update = opt_update_func.value();
    return true;
}

bool Game::load_shaders() {
    auto& passet_manager = pregistry_->get_context<AssetManagerPtr>();
    if (!passet_manager->add_shader_from_memory("basic", DefaultShaders::basic_shader_vert,
                                                DefaultShaders::basic_shader_frag)) {
        std::cerr << "could not load basic shaders from memory\n";
        return false;
    }

    if (!passet_manager->add_shader_from_memory("font", DefaultShaders::font_shader_vert,
                                                DefaultShaders::font_shader_frag)) {
        std::cerr << "could not font shaders from memory\n";
        return false;
    }

    return true;
}

bool Game::init_sdl() {
    std::cout << "initializing sdl\n";
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "sdl initialization failed: " << SDL_GetError() << "\n";
        return EXIT_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    pwindow_ = SDL_CreateWindow("sdl and emscripten test", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);

    if (pwindow_ == nullptr) {
        std::cerr << "failed to create sdl window: " << SDL_GetError() << "\n";
    }

    pglcontext_ = SDL_GL_CreateContext(pwindow_);
    // enable vsync
    SDL_GL_SetSwapInterval(1);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

    std::cout << "sdl initialized succesfully\n";

    return true;
}

void Game::register_meta_components() {
    Entity::register_meta_component<Identification>();
    Entity::register_meta_component<TransformComponent>();
    Entity::register_meta_component<SpriteComponent>();
    Entity::register_meta_component<BoxColider>();
    Entity::register_meta_component<CircleCollider>();
    Entity::register_meta_component<AnimationComponent>();
    Entity::register_meta_component<TransformComponent>();
    Entity::register_meta_component<RigidBodyComponent>();
    Entity::register_meta_component<TextComponent>();

    Registry::register_meta_component<Identification>();
    Registry::register_meta_component<TransformComponent>();
    Registry::register_meta_component<SpriteComponent>();
    Registry::register_meta_component<BoxColider>();
    Registry::register_meta_component<CircleCollider>();
    Registry::register_meta_component<AnimationComponent>();
    Registry::register_meta_component<TransformComponent>();
    Registry::register_meta_component<RigidBodyComponent>();
    Registry::register_meta_component<TextComponent>();
}

void Game::register_lua_bindings() {
    auto& plua_state = pregistry_->get_context<SolStatePtr>();
    auto& pcamera = pregistry_->get_context<CameraPtr>();
    auto& passet_manager = pregistry_->get_context<AssetManagerPtr>();
    auto& pinput_context = pregistry_->get_context<InputCtxPtr>();
    auto& paudio_context = pregistry_->get_context<AudioCtxPtr>();

    AssetManager::create_lua_bind(*plua_state, *passet_manager);
    Camera::create_lua_bind(*plua_state, *pcamera);
    Vertex::create_lua_bind(*plua_state);
    GlmBinder::create_lua_bind(*plua_state);
    ComponentBinder::create_lua_bind(*plua_state);
    Registry::create_lua_bind(*plua_state, *pregistry_);
    Entity::create_lua_bind(*plua_state, *pregistry_);
    Keyboard::create_lua_bind(*plua_state, *pinput_context->pkeyboard_);
    Mouse::create_lua_bind(*plua_state, *pinput_context->pmouse_);
    Gamepad::create_lua_bind(*plua_state, *pinput_context->pgamepad_);
    ScriptFuncBinder::create_lua_bind(*plua_state);
    MusicPlayer::create_lua_bind(*plua_state, *paudio_context->pmusic_player_, *passet_manager);
    SoundPlayer::create_lua_bind(*plua_state, *paudio_context->psound_player_, *passet_manager);
}

void Game::process_events() {
    auto& pinput_context = pregistry_->get_context<InputCtxPtr>();

    while (SDL_PollEvent(&event_)) {
        switch (event_.type) {

            case SDL_KEYDOWN:
                pinput_context->pkeyboard_->on_key_pressed(event_.key.keysym.sym);
                break;

            case SDL_KEYUP:
                pinput_context->pkeyboard_->on_key_released(event_.key.keysym.sym);
                break;

            case SDL_MOUSEBUTTONDOWN:
                pinput_context->pmouse_->on_btn_pressed(event_.button.button);
                break;

            case SDL_MOUSEBUTTONUP:
                pinput_context->pmouse_->on_btn_released(event_.button.button);
                break;

            case SDL_MOUSEWHEEL:
                pinput_context->pmouse_->set_mouse_wheel_values(event_.wheel.x, event_.wheel.y);
                break;

            case SDL_MOUSEMOTION:
                pinput_context->pmouse_->set_mouse_moving(true);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                pinput_context->pgamepad_->on_btn_pressed(event_.cbutton.button);
                break;

            case SDL_CONTROLLERBUTTONUP:
                pinput_context->pgamepad_->on_btn_released(event_.cbutton.button);
                break;

            case SDL_CONTROLLERDEVICEADDED: {
                std::cout << "controller detected: " << event_.jdevice.which << "\n";
                if (!pinput_context->pgamepad_->is_game_pad_present()) {
                    pinput_context->pgamepad_->set_controller(
                        make_shared_controller(SDL_GameControllerOpen(event_.jdevice.which)));
                }

                break;
            }

            case SDL_CONTROLLERDEVICEREMOVED: {
                if (pinput_context->pgamepad_->is_game_pad_present()) {
                    pinput_context->pgamepad_->remove_controller();
                }

                break;
            }

            case SDL_JOYAXISMOTION:
                pinput_context->pgamepad_->set_axis_position_value(event_.jaxis.axis,
                                                                   event_.jaxis.value);
                break;

            case SDL_JOYHATMOTION:
                pinput_context->pgamepad_->set_joystick_hat_value(event_.jhat.value);
                break;

            case SDL_QUIT:
#ifdef __EMSCRIPTEN__
                emscripten_cancel_main_loop();
#endif
                break;

            default:
                break;
        }
    }
}

void Game::update() {
    if (main_script_.update.valid()) {
        auto result = main_script_.update();
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "lua update error: " << err.what() << "\n";
        }
    }

    pregistry_->get_context<InputCtxPtr>()->update();
    pregistry_->get_context<CameraPtr>()->update();
}

void Game::render() {
    SDL_GL_MakeCurrent(pwindow_, pglcontext_);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int w, h;
    SDL_GetWindowSize(pwindow_, &w, &h);
    glViewport(0.0f, 0.0f, w, h);

    render_sprites();
    render_text();

    SDL_GL_SwapWindow(pwindow_);
}

void Game::render_text() {
    auto& pcamera = pregistry_->get_context<CameraPtr>();
    auto& ptext_batch_renderer = pregistry_->get_context<TextBatchRendererPtr>();
    auto& passet_manager = pregistry_->get_context<AssetManagerPtr>();
    auto pfont_shader = passet_manager->get_shader("font");

    if (!pfont_shader) {
        std::cerr << "could not find font shader\n";
        return;
    }

    auto text_camera_matrix = pcamera->get_camera_matrix();
    auto text_view = pregistry_->get_registry().view<TextComponent>();

    ptext_batch_renderer->begin();
    pfont_shader->enable();
    pfont_shader->set_uniform_mat4("u_projection", text_camera_matrix);

    for (auto entity : text_view) {
        Entity ent{*pregistry_, entity};
        const auto& text = ent.get_component<TextComponent>();
        if (text.hidden_) {
            continue;
        }
        auto pfont = passet_manager->get_font(text.font_name_);
        if (!pfont) {
            continue;
        }
        const auto& transform = ent.get_component<TransformComponent>();
        ptext_batch_renderer->add_text(text.text_, pfont, transform.position_, text.color_);
    }

    ptext_batch_renderer->end();
    ptext_batch_renderer->render();
    pfont_shader->disable();
}

void Game::render_sprites() {
    auto& pcamera = pregistry_->get_context<CameraPtr>();
    auto& pbatch_renderer = pregistry_->get_context<BatchRendererPtr>();
    auto& passet_manager = pregistry_->get_context<AssetManagerPtr>();
    auto pshader = passet_manager->get_shader("basic");

    if (!pshader) {
        std::cerr << "failed to render sprites, shader does not exist\n";
        return;
    }

    pshader->enable();
    auto cam_mat = pcamera->get_projection_matrix();
    pshader->set_uniform_mat4("u_projection", cam_mat);

    auto view = pregistry_->get_registry().view<TransformComponent, SpriteComponent>();
    pbatch_renderer->begin();

    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        const auto& sprite = view.get<SpriteComponent>(entity);
        auto ptexture = passet_manager->get_texture(sprite.string_);

        if (!ptexture || sprite.hidden_) {
            continue;
        }

        glm::vec4 pos{transform.position_.x, transform.position_.y,
                      sprite.width_ * transform.scale_.x, sprite.height_ * transform.scale_.y};

        glm::vec4 uvs{sprite.uvs_.u_, sprite.uvs_.v_, sprite.uvs_.uv_widht_,
                      sprite.uvs_.uv_height_};

        pbatch_renderer->add_sprite(pos, uvs, sprite.layer_, ptexture->get_id(), sprite.color_);
    }

    pbatch_renderer->end();
    pbatch_renderer->render();
    pshader->disable();
}

void Game::cleanup() {
    SDL_GL_DeleteContext(pglcontext_);
    SDL_DestroyWindow(pwindow_);
    SDL_Quit();
}
