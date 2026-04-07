#pragma once
#include <SDL2/SDL_mixer.h>
#include <sol/sol.hpp>
#include <sol/state.hpp>

namespace jpengine {
class SoundPlayer {
public:
    SoundPlayer() = default;
    ~SoundPlayer() = default;

    void play(Mix_Chunk* pchunk, int loops = 0, int channel = -1);
    void set_volume(float volume, int channel = -1);
    void stop(int channel);
    bool is_playing(int channel);

    static void create_lua_bind(sol::state& lua, SoundPlayer& sound_player,
                                class AssetManager& asset_manager);
};
} // namespace jpengine
