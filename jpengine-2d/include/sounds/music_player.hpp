#pragma once
#include <SDL2/SDL_mixer.h>
#include <sol/sol.hpp>
#include <sol/state.hpp>

namespace jpengine {

class MusicPlayer {
public:
    MusicPlayer();
    MusicPlayer(int frequency, Uint16 format, int channels, int chunksize, int allow_changes);
    ~MusicPlayer();
    void play(Mix_Music* pmusic, int loops = 0);
    void pause();
    void resume();
    void stop();
    void set_volume(float volume);
    [[nodiscard]] bool is_playing() const noexcept;
    static void create_lua_bind(sol::state& lua, MusicPlayer& music_player,
                                class AssetManager& asset_manager);
};

} // namespace jpengine
