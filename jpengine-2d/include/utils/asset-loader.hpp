#pragma once

#include <SDL2/SDL_mixer.h>
#include <memory>
#include <string_view>

namespace jpengine {
class Shader;
class Texture;
class Font;

} // namespace jpengine

namespace jpengine::utils {
class AssetLoader {
public:
    AssetLoader() = delete;
    static std::shared_ptr<jpengine::Shader> load_shader_from_memory(const char* vertex_shader,
                                                                     const char* frag_shader);
    static std::shared_ptr<jpengine::Texture> load_texture(std::string_view filename,
                                                           bool pixel_art);
    static std::shared_ptr<jpengine::Font> load_font(std::string_view filename,
                                                     float font_size = 32.F);
    static Mix_Music* load_music(std::string_view filename);
    static Mix_Chunk* load_soundfx(std::string_view filename);
};
} // namespace jpengine::utils
