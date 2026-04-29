#include "engine/src/graphics/texture.hpp"

#include "utils/asset-path.hpp"

#include <GL/glew.h>
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
    // Force RGBA on load → always upload as RGBA8.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

std::shared_ptr<Texture> Texture::load(const std::string& path) {
    const auto resolved = utils::asset_path(path);
    if (!std::filesystem::exists(resolved)) {
        return nullptr;
    }

    // OpenGL UV origin is bottom-left; PNGs are stored top-down. Flip on load.
    stbi_set_flip_vertically_on_load(static_cast<int>(true));

    int width = 0;
    int height = 0;
    int numchannels = 0;
    unsigned char* data = stbi_load(resolved.string().c_str(), &width, &height, &numchannels,
                                    4); // force RGBA
    if (data == nullptr) {
        return nullptr;
    }

    auto texture = std::make_shared<Texture>(width, height, 4, data);
    stbi_image_free(data);
    return texture;
}
} // namespace engine
