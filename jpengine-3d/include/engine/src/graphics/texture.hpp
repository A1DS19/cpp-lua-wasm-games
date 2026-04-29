#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace engine {

// Avoid pulling <GL/glew.h> into a public header — GLEW must be included before
// any other GL header, and dragging it through the include graph means every
// consumer of this file inherits that constraint. GLuint is `unsigned int` per
// the GL spec, so we can use the underlying type directly.
using TextureHandle = unsigned int;

class Texture {

public:
    Texture(int width, int height, int numchannels, unsigned char* data);
    ~Texture();
    [[nodiscard]] TextureHandle get_id() const { return texture_id_; }

    void init(int width, int height, int numchannels, unsigned char* data);
    static std::shared_ptr<class Texture> load(const std::string& path);

private:
    TextureHandle texture_id_ = 0;
    int width_ = 0;
    int height_ = 0;
    int numchannels_ = 0;
};

class TextureManager {
public:
    std::shared_ptr<Texture> get_or_load_texture(const std::string& path) {
        auto it = textures_.find(path);
        if (it != textures_.end()) {
            return it->second;
        }

        auto texture = Texture::load(path);
        textures_[path] = texture;

        return texture;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;
};

} // namespace engine
