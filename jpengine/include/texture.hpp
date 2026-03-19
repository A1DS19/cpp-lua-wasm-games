#pragma once

#include <string_view>
#ifdef __APPLE__
    #include <OpenGL/gl3.h>
#else
    #include <GLES3/gl3.h>
#endif

namespace jpengine {
class Texture {
public:
    Texture();
    Texture(GLuint id, int width, int height, std::string_view texture_path);
    void enable();
    void disable();
    [[nodiscard]] int get_width() const noexcept { return width_; };
    [[nodiscard]] int get_height() const noexcept { return height_; }
    [[nodiscard]] GLuint get_id() const { return texture_id_; }

private:
    GLuint texture_id_;
    int width_, height_;
    std::string_view texture_path_;
};
} // namespace jpengine
