#include "engine/src/graphics/texture.hpp"

#include "utils/asset-path.hpp"

#include <filesystem>
#include <memory>
#include <stb/stb_image.h>

namespace engine {
Texture::Texture(int width, int height, int numchannels, unsigned char* data)
    : width_{width}, height_{height}, numchannels_{numchannels} {
    init(width, height, numchannels, data);
}

Texture::~Texture() {
    if (texture_id_ > 0) {
        glDeleteTextures(1, &texture_id_);
    }
}

void Texture::init(int width, int height, int numchannels, unsigned char* data) {
    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

std::shared_ptr<Texture> Texture::load(const std::string& path) {
    int width, height, numchannels;

    if (!std::filesystem::exists(utils::asset_path(path))) {
        return nullptr;
    }

    std::shared_ptr<engine::Texture> texture;

    unsigned char* data =
        stbi_load(utils::asset_path_str("brick.png").c_str(), &width, &height, &numchannels, 0);

    if (static_cast<bool>(data)) {
        texture = std::make_shared<Texture>(width, height, numchannels, data);
        stbi_image_free(data);
    }

    return texture;
}
} // namespace engine
