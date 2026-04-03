#include "rendering/text-batch-renderer.hpp"

#include "rendering/font.hpp"
#include "rendering/vertex.hpp"

#include <GLES3/gl3.h>
#include <cstddef>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <memory>
#include <vector>

using namespace jpengine;

namespace jpengine {
constexpr int MAX_VERTICES = 24000;
constexpr int NUM_VERTICES = 6;
}; // namespace jpengine

TextBatchRenderer::TextBatchRenderer() : vao_{0}, vbo_{0}, text_glyphs_{}, text_batches_{} {
    init();
}
TextBatchRenderer::~TextBatchRenderer() {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
}

void TextBatchRenderer::begin() {
    text_batches_.clear();
    text_glyphs_.clear();
}

void TextBatchRenderer::end() {
    if (text_glyphs_.empty()) {
        return;
    }

    create_batches();
}

void TextBatchRenderer::add_text(std::string_view text, std::shared_ptr<Font>& pfont,
                                 const glm::vec2& position, const Color& color,
                                 const glm::mat4& model) {
    text_glyphs_.emplace_back(std::make_unique<TextGlyph>(TextGlyph{.text_ = text.data(),
                                                                    .position_ = position,
                                                                    .color_ = color,
                                                                    .model_ = model,
                                                                    .pfont_ = pfont}));
}

void TextBatchRenderer::render() {
    if (text_batches_.empty()) {
        return;
    }

    glBindVertexArray(vao_);
    for (const auto& batch : text_batches_) {
        glBindTexture(GL_TEXTURE_2D, batch->texture_id_);
        glDrawArrays(GL_TRIANGLES, static_cast<GLint>(batch->offset_),
                     static_cast<GLsizei>(batch->num_vertices_));
    }
}

void TextBatchRenderer::init() {
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position_)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uvs_)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, color_)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void TextBatchRenderer::create_batches() {
    GLuint offset{0};
    GLuint prev_font_id{0};

    int current_font{0};

    size_t total{0};
    size_t current_vertex{0};

    for (const auto& text_glyph : text_glyphs_) {
        total += text_glyph->text_.size();
    }
    std::vector<Vertex> vertices;
    vertices.resize(total * NUM_VERTICES);

    for (const auto& text_glyph : text_glyphs_) {
        glm::vec2 temp_pos = text_glyph->position_;
        for (const auto& character : text_glyph->text_) {
            auto glyph = text_glyph->pfont_->get_glyph(character, temp_pos);

            // first TRIANGLE
            vertices[current_vertex++] = Vertex{
                .position_ = text_glyph->model_ *
                             glm::vec4{glyph.min_.position_.x, glyph.min_.position_.y, 0.F, 1.F},
                .uvs_ = UV{.u_ = glyph.min_.uvs_.u_, .v_ = glyph.min_.uvs_.v_},
                .color_ = text_glyph->color_};

            vertices[current_vertex++] = Vertex{
                .position_ = text_glyph->model_ *
                             glm::vec4{glyph.max_.position_.x, glyph.min_.position_.y, 0.F, 1.F},
                .uvs_ = UV{.u_ = glyph.max_.uvs_.u_, .v_ = glyph.min_.uvs_.v_},
                .color_ = text_glyph->color_};

            vertices[current_vertex++] = Vertex{
                .position_ = text_glyph->model_ *
                             glm::vec4{glyph.max_.position_.x, glyph.max_.position_.y, 0.F, 1.F},
                .uvs_ = UV{.u_ = glyph.max_.uvs_.u_, .v_ = glyph.max_.uvs_.v_},
                .color_ = text_glyph->color_};

            // second TRIANGLE
            vertices[current_vertex++] = Vertex{
                .position_ = text_glyph->model_ *
                             glm::vec4{glyph.min_.position_.x, glyph.min_.position_.y, 0.F, 1.F},
                .uvs_ = UV{.u_ = glyph.min_.uvs_.u_, .v_ = glyph.min_.uvs_.v_},
                .color_ = text_glyph->color_};

            vertices[current_vertex++] = Vertex{
                .position_ = text_glyph->model_ *
                             glm::vec4{glyph.max_.position_.x, glyph.max_.position_.y, 0.F, 1.F},
                .uvs_ = UV{.u_ = glyph.max_.uvs_.u_, .v_ = glyph.max_.uvs_.v_},
                .color_ = text_glyph->color_};

            vertices[current_vertex++] = Vertex{
                .position_ = text_glyph->model_ *
                             glm::vec4{glyph.min_.position_.x, glyph.max_.position_.y, 0.F, 1.F},
                .uvs_ = UV{.u_ = glyph.min_.uvs_.u_, .v_ = glyph.max_.uvs_.v_},
                .color_ = text_glyph->color_};

            if (current_font == 0) {
                text_batches_.emplace_back(std::make_unique<TextBatch>(
                    TextBatch{.num_vertices_ = NUM_VERTICES,
                              .texture_id_ = text_glyph->pfont_->get_font_atlas_id(),
                              .offset_ = offset}));
            } else if (text_glyph->pfont_->get_font_atlas_id() != prev_font_id) {
                text_batches_.emplace_back(std::make_unique<TextBatch>(
                    TextBatch{.num_vertices_ = NUM_VERTICES,
                              .texture_id_ = text_glyph->pfont_->get_font_atlas_id(),
                              .offset_ = offset}));
            } else {
                text_batches_.back()->num_vertices_ += NUM_VERTICES;
            }

            current_font++;
            prev_font_id = text_glyph->pfont_->get_font_atlas_id();
            offset += NUM_VERTICES;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    auto buffer_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex));
    glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
