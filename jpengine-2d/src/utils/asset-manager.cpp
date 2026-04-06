#include "utils/asset-manager.hpp"

#include "rendering/font.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"
#include "utils/asset-loader.hpp"

#include <iostream>

using namespace jpengine;

AssetManager::~AssetManager() {
    clear();
}

bool AssetManager::add_texture(const std::string& texture_name, const std::string& filename,
                               bool pixel_art) {
    if (map_textures_.contains(texture_name)) {
        std::cerr << "failed to add texture" << texture_name << ", it already exists\n";
        return false;
    }

    auto ptexture = utils::AssetLoader::load_texture(filename, pixel_art);
    if (!ptexture) {
        std::cerr << "failed to load texture: " << texture_name << "\n";
        return false;
    }

    auto [iter, inserted] = map_textures_.emplace(texture_name, std::move(ptexture));
    return inserted;
}

std::shared_ptr<Texture> AssetManager::get_texture(const std::string& texture_name) {
    auto texture_iter = map_textures_.find(texture_name);
    if (texture_iter == map_textures_.end()) {
        std::cerr << "failed to find: " << texture_name << "\n";
        return nullptr;
    }

    return texture_iter->second;
}

bool AssetManager::add_shader_from_memory(const std::string& shader_name,
                                          const std::string& vert_data,
                                          const std::string& frag_data) {
    if (map_shaders_.contains(shader_name)) {
        std::cerr << "failed to add shader" << shader_name << ", it already exists\n";
        return false;
    }

    auto pshader =
        utils::AssetLoader::load_shader_from_memory(vert_data.c_str(), frag_data.c_str());
    if (!pshader) {
        std::cerr << "failed to load shader: " << shader_name << "\n";
        return false;
    }

    auto [iter, inserted] = map_shaders_.emplace(shader_name, std::move(pshader));
    return inserted;
}

std::shared_ptr<Shader> AssetManager::get_shader(const std::string& shader_name) {
    auto shader_iter = map_shaders_.find(shader_name);
    if (shader_iter == map_shaders_.end()) {
        std::cerr << "failed to find: " << shader_name << "\n";
        return nullptr;
    }

    return shader_iter->second;
}

bool AssetManager::add_font(const std::string& font_name, const std::string& filename,
                            float fontsize) {
    if (map_fonts_.contains(font_name)) {
        std::cerr << "failed to add font" << font_name << ", it already exists\n";
        return false;
    }

    auto pfont = utils::AssetLoader::load_font(filename, fontsize);
    if (!pfont) {
        std::cerr << "failed to load font: " << font_name << "\n";
        return false;
    }

    auto [iter, inserted] = map_fonts_.emplace(font_name, std::move(pfont));
    return inserted;
}

std::shared_ptr<Font> AssetManager::get_font(const std::string& font_name) {
    auto font_iter = map_fonts_.find(font_name);
    if (font_iter == map_fonts_.end()) {
        std::cerr << "failed to find: " << font_name << "\n";
        return nullptr;
    }

    return font_iter->second;
}

bool AssetManager::add_music(const std::string& music_name, const std::string& filename) {
    if (map_music_.contains(music_name)) {
        std::cerr << "failed to add music" << music_name << ", it already exists\n";
        return false;
    }

    auto pmusic = utils::AssetLoader::load_music(filename);
    if (!pmusic) {
        std::cerr << "failed to load music: " << music_name << "\n";
        return false;
    }

    auto [iter, inserted] = map_music_.emplace(music_name, std::move(pmusic));
    return inserted;
}

Mix_Music* AssetManager::get_music(const std::string& music_name) {
    auto music_iter = map_music_.find(music_name);
    if (music_iter == map_music_.end()) {
        std::cerr << "failed to find: " << music_name << "\n";
        return nullptr;
    }

    return music_iter->second;
}

bool AssetManager::add_soundfx(const std::string& soundfx_name, const std::string& filename) {
    if (map_soundfxs_.contains(soundfx_name)) {
        std::cerr << "failed to add soundfx" << soundfx_name << ", it already exists\n";
        return false;
    }

    auto pchunk = utils::AssetLoader::load_soundfx(filename);
    if (!pchunk) {
        std::cerr << "failed to load soundfx: " << soundfx_name << "\n";
        return false;
    }

    auto [iter, inserted] = map_soundfxs_.emplace(soundfx_name, pchunk);
    return inserted;
}

Mix_Chunk* AssetManager::get_soundfx(const std::string& soundfx_name) {
    auto soundfx_iter = map_soundfxs_.find(soundfx_name);
    if (soundfx_iter == map_soundfxs_.end()) {
        std::cerr << "failed to find: " << soundfx_name << "\n";
        return nullptr;
    }

    return soundfx_iter->second;
}

bool AssetManager::clear() {
    for (auto& [_, pmusic] : map_music_) {
        Mix_FreeMusic(pmusic);
    }

    for (auto& [_, psoundfx] : map_soundfxs_) {
        Mix_FreeChunk(psoundfx);
    }

    map_textures_.clear();
    map_fonts_.clear();
    map_shaders_.clear();
    map_music_.clear();
    map_soundfxs_.clear();

    return true;
}

void AssetManager::create_lua_bind(sol::state& lua, AssetManager& asset_manager) {
    (void)asset_manager;

    lua.new_usertype<AssetManager>(
        "AssetManager", sol::no_constructor, "add_texture", &AssetManager::add_texture,
        "get_texture", &AssetManager::get_texture, "add_music", &AssetManager::add_music,
        "add_soundfx", &AssetManager::add_soundfx, "add_font", &AssetManager::add_font, "get_font",
        &AssetManager::get_font);
}
