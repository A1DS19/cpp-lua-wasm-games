#include "texture.hpp"
using namespace jpengine;

Texture::Texture() : Texture(0, 0, 0, "") {}

Texture::Texture(GLuint id, int width, int height, std::string_view texture_path)
    : texture_id_(id), width_(width), height_(height), texture_path_(texture_path) {}

void Texture::enable() {
    glBindTexture(GL_TEXTURE_2D, texture_id_);
}

void Texture::disable() {
    glBindTexture(GL_TEXTURE_2D, 0);
}
