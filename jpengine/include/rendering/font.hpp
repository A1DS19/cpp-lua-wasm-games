#pragma once

#include "vertex.hpp"

#include <GLES3/gl3.h>
#include <glm/ext/vector_float2.hpp>

namespace jpengine {
struct FontGlyph {
    Vertex min_;
    Vertex max_;
};

class Font {

public:
    Font(GLuint font_id, int width, int height, float font_size, void* pdata);
    ~Font();

    FontGlyph get_glyph(char c, glm::vec2& pos);
    void get_next_char_pos(char c, glm::vec2& pos);

    [[nodiscard]] GLuint get_font_atlas_id() const noexcept { return font_atlas_id_; }
    [[nodiscard]] float get_font_size() const noexcept { return font_size_; }

private:
    GLuint font_atlas_id_;
    int width_;
    int height_;
    float font_size_;
    void* pfont_data_;
};
} // namespace jpengine
