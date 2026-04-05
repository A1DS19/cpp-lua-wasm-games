#include "sounds/sound_player.hpp"

#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <iostream>

using namespace jpengine;

void SoundPlayer::play(Mix_Chunk* pchunk, int loops, int channel) {
    if (!pchunk) {
        return;
    }

    if (Mix_PlayChannel(channel, pchunk, loops) == -1) {
        std::cerr << "failed to play sound: " << Mix_GetError() << "\n";
    }
}

void SoundPlayer::set_volume(float volume, int channel) {
    volume = std::clamp(volume, 0.F, 1.F);
    int final_volume = static_cast<int>(MIX_MAX_VOLUME * volume);
    Mix_Volume(channel, final_volume);
}

void SoundPlayer::stop(int channel) {
    Mix_HaltChannel(channel);
}

bool SoundPlayer::is_playing(int channel) {
    return Mix_Playing(channel);
}

void SoundPlayer::create_lua_bind(sol::state& lua, SoundPlayer& sound_player) {
    (void)lua;
    (void)sound_player;
}
