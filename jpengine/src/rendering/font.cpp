#include "rendering/font.hpp"

#include "rendering/vertex.hpp"

#include <GLES3/gl3.h>
#include <glm/ext/vector_float2.hpp>
#include <stb/stb_truetype.h>

using namespace jpengine;

Font::Font(GLuint font_id, int width, int height, float font_size, void* pdata)
    : font_atlas_id_{font_id}, width_{width}, height_{height}, font_size_{font_size},
      pfont_data_{std::move(pdata)} {}

Font::~Font() {
    if (font_atlas_id_ != 0) {
        glDeleteTextures(1, &font_atlas_id_);
    }

    if (pfont_data_) {
        delete[] static_cast<stbtt_bakedchar*>(pfont_data_);
    }
}

FontGlyph Font::get_glyph(char c, glm::vec2& pos) {
    FontGlyph glyph{};

    if (c >= 32 && static_cast<unsigned char>(c) < 128) {
        stbtt_aligned_quad quad;
        stbtt_GetBakedQuad(static_cast<stbtt_bakedchar*>(pfont_data_), width_, height_, c - 32,
                           &pos.x, &pos.y, &quad, 1);

        glyph.min_ = Vertex{
            .position_ = glm::vec2{quad.x0, quad.y0},
            .uvs_ = UV{.u_ = quad.s0, .v_ = quad.t0},
        };

        glyph.max_ = Vertex{
            .position_ = glm::vec2{quad.x1, quad.y1},
            .uvs_ = UV{.u_ = quad.s1, .v_ = quad.t1},
        };
    }

    return glyph;
}

void Font::get_next_char_pos(char c, glm::vec2& pos) {

    if (c >= 32 && static_cast<unsigned char>(c) < 128) {
        stbtt_aligned_quad quad;
        stbtt_GetBakedQuad(static_cast<stbtt_bakedchar*>(pfont_data_), width_, height_, c - 32,
                           &pos.x, &pos.y, &quad, 1);
    }
}
