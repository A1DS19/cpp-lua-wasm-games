#include "sounds/sound_player.hpp"

#include "utils/asset-manager.hpp"

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

void SoundPlayer::create_lua_bind(sol::state& lua, SoundPlayer& sound_player,
                                  AssetManager& asset_manager) {
    lua.new_usertype<SoundPlayer>(
        "Sound", sol::no_constructor, "play",
        sol::overload(
            [&](const std::string& soundName) {
                auto pSoundFx = asset_manager.get_soundfx(soundName);
                if (!pSoundFx) {
                    std::cerr << "Failed to get [" << soundName << "] from the Asset Manager\n";
                    return;
                }

                sound_player.play(pSoundFx);
            },
            [&](const std::string& soundName, int loops, int channel) {
                auto pSoundFx = asset_manager.get_soundfx(soundName);
                if (!pSoundFx) {
                    std::cerr << "Failed to get [" << soundName << "] from the Asset Manager.\n";
                    return;
                }

                sound_player.play(pSoundFx, loops, channel);
            }),
        "stop", [&](int channel) { sound_player.stop(channel); }, "set_volume",
        [&](int channel, int volume) { sound_player.set_volume(channel, volume); }, "is_playing",
        [&](int channel) { return sound_player.is_playing(channel); });
}
