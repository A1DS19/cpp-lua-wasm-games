#pragma once

#include "physics/contact-listener.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL_mixer.h>
#include <memory>
#include <sol/sol.hpp>

namespace jpengine {

class Shader;
class Texture;
class Font;
class Registry;
class Keyboard;
class Mouse;
class Camera;
class Gamepad;
class BatchRenderer;
class TextBatchRenderer;
class MusicPlayer;
class SoundPlayer;
class AssetManager;

struct MainGameScripts {
    sol::protected_function update{sol::lua_nil_t{}};
    sol::protected_function render{sol::lua_nil_t{}};
};

struct AudioContext {
    std::shared_ptr<MusicPlayer> pmusic_player_{nullptr};
    std::shared_ptr<SoundPlayer> psound_player_{nullptr};
};

struct InputContext {
    std::shared_ptr<Keyboard> pkeyboard_{nullptr};
    std::shared_ptr<Mouse> pmouse_{nullptr};
    std::shared_ptr<Gamepad> pgamepad_{nullptr};

    void update();
};

using InputCtxPtr = std::shared_ptr<InputContext>;
using SolStatePtr = std::shared_ptr<sol::state>;
using CameraPtr = std::shared_ptr<Camera>;
using BatchRendererPtr = std::shared_ptr<BatchRenderer>;
using TextBatchRendererPtr = std::shared_ptr<TextBatchRenderer>;
using AudioCtxPtr = std::shared_ptr<AudioContext>;
using AssetManagerPtr = std::shared_ptr<AssetManager>;
using ContactListenerPtr = std::shared_ptr<ContactListener>;

constexpr float ONE_OVER_SIXTY = 1.F / 60.F;

class Game {
public:
    Game();
    ~Game();

    void run();
    bool initialize();

private:
    SDL_Window* pwindow_;
    SDL_GLContext pglcontext_;
    std::unique_ptr<Registry> pregistry_;
    MainGameScripts main_script_;
    bool runnning_;
    SDL_Event event_;

    bool initialize_registry();
    bool load_main_script();
    bool load_shaders();

    bool init_sdl();
    void register_meta_components();
    void register_lua_bindings();

    void process_events();
    void update();
    void update_physics();
    void render();

    void render_text();
    void render_sprites();
    void cleanup();
};

} // namespace jpengine
