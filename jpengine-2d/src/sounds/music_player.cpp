#include "sounds/music_player.hpp"

#include "utils/asset-manager.hpp"

#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_stdinc.h>
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

using namespace jpengine;

namespace jpengine {
constexpr int DEFAULT_FREQUENCY = 44100;
constexpr int DEFAULT_CHANNELS = 2;
constexpr int DEFAULT_CHUNKSIZE = 1152;
constexpr int DEFAULT_MIXER_FLAGS =
    SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE;
constexpr Uint16 DEFAULT_FORMAT = MIX_DEFAULT_FORMAT;
} // namespace jpengine

MusicPlayer::MusicPlayer()
    : MusicPlayer(DEFAULT_FREQUENCY, DEFAULT_FORMAT, DEFAULT_CHANNELS, DEFAULT_CHUNKSIZE,
                  DEFAULT_MIXER_FLAGS) {}

MusicPlayer::MusicPlayer(int frequency, Uint16 format, int channels, int chunksize,
                         int allow_changes) {
    if (Mix_OpenAudioDevice(frequency, format, channels, chunksize, NULL, allow_changes) == -1) {
        std::string error = Mix_GetError();
        std::cerr << "failed to open audio device: " << error << "\n";
        return;
    }

    Mix_Init(MIX_INIT_OGG);
}

MusicPlayer::~MusicPlayer() {
    Mix_HaltMusic();
    Mix_CloseAudio();
    Mix_Quit();
}

void MusicPlayer::play(Mix_Music* pmusic, int loops) {
    if (!pmusic) {
        return;
    }

    if (Mix_PlayMusic(pmusic, loops) == -1) {
        std::cerr << "failed to play music: " << Mix_GetError() << "\n";
    }
}

void MusicPlayer::pause() {
    Mix_PauseMusic();
}

void MusicPlayer::resume() {
    Mix_ResumeMusic();
}

void MusicPlayer::stop() {
    Mix_HaltMusic();
}

void MusicPlayer::set_volume(float volume) {
    volume = std::clamp(volume, 0.F, 1.F);
    int final_volume = static_cast<int>(MIX_MAX_VOLUME * volume);
    Mix_VolumeMusic(final_volume);
}

[[nodiscard]] bool MusicPlayer::is_playing() const noexcept {
    return Mix_PlayingMusic();
}

void MusicPlayer::create_lua_bind(sol::state& lua, MusicPlayer& music_player,
                                  AssetManager& asset_manager) {

    lua.new_usertype<MusicPlayer>(
        "Music", sol::no_constructor, "play",
        sol::overload(
            [&](const std::string& musicName, int loops) {
                auto pMusic = asset_manager.get_music(musicName);
                if (!pMusic) {
                    std::cerr << "Failed to get music [" << musicName
                              << "] - From the asset maanger!\n";
                    return;
                }
                music_player.play(pMusic, loops);
            },
            [&](const std::string& musicName) {
                auto pMusic = asset_manager.get_music(musicName);
                if (!pMusic) {
                    std::cerr << "Failed to get music [" << musicName
                              << "] - From the asset maanger!\n";
                    return;
                }
                music_player.play(pMusic, -1);
            }),
        "stop", [&]() { music_player.stop(); }, "pause", [&]() { music_player.pause(); }, "resume",
        [&]() { music_player.resume(); }, "set_volume",
        [&](int volume) { music_player.set_volume(volume); }, "is_playing",
        [&]() { return music_player.is_playing(); });
    lua["MusicPlayer"] = &music_player;
}
