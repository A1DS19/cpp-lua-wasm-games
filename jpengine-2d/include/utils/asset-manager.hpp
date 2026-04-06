#pragma once

#include <SDL2/SDL_mixer.h>
#include <memory>
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>

namespace jpengine {

class Texture;
class Shader;
class Font;

class AssetManager {

public:
    AssetManager() = default;
    ~AssetManager();

    bool add_texture(const std::string& texture_name, const std::string& filename,
                     bool pixel_art = true);
    std::shared_ptr<Texture> get_texture(const std::string& texture_name);

    bool add_shader_from_memory(const std::string& shader_name, const std::string& vert_data,
                                const std::string& frag_data);
    std::shared_ptr<Shader> get_shader(const std::string& shader_name);

    bool add_font(const std::string& font_name, const std::string& filename, float fontsize = 32.F);
    std::shared_ptr<Font> get_font(const std::string& font_name);

    bool add_music(const std::string& music_name, const std::string& filename);
    Mix_Music* get_music(const std::string& music_name);

    bool add_soundfx(const std::string& soundfx_name, const std::string& filename);
    Mix_Chunk* get_soundfx(const std::string& soundfx_name);

    bool clear();

    static void create_lua_bind(sol::state& lua, AssetManager& asset_manager);

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> map_textures_;
    std::unordered_map<std::string, std::shared_ptr<Shader>> map_shaders_;
    std::unordered_map<std::string, std::shared_ptr<Font>> map_fonts_;
    std::unordered_map<std::string, Mix_Music*> map_music_;
    std::unordered_map<std::string, Mix_Chunk*> map_soundfxs_;
};

} // namespace jpengine
