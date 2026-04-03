#include "rendering/batch-renderer.hpp"

#include "rendering/shader.hpp"
#include "rendering/vertex.hpp"

#include <algorithm>
#include <cstddef>
#include <glm/ext/vector_float4.hpp>
#include <vector>

using namespace jpengine;

namespace jpengine {
constexpr std::size_t NUM_SPRITE_VERTICES = 4;
constexpr std::size_t NUM_SPRITE_INDICES = 6;
constexpr std::size_t MAX_SPRITES = 10000;
constexpr std::size_t MAX_INDICES = MAX_SPRITES * NUM_SPRITE_INDICES;
constexpr std::size_t MAX_VERTICES = MAX_SPRITES * NUM_SPRITE_VERTICES;
} // namespace jpengine

BatchRenderer::BatchRenderer() : vao_{0}, vbo_{0}, ibo_{0}, sprite_glyphs_{}, batches_{} {
    init();
}

BatchRenderer::~BatchRenderer() {
    if (vao_ > 0) {
        glDeleteVertexArrays(1, &vao_);
    }

    if (vbo_ > 0) {
        glDeleteBuffers(1, &vbo_);
    }

    if (ibo_ > 0) {
        glDeleteBuffers(1, &ibo_);
    }
}

void BatchRenderer::begin() {
    batches_.clear();
    sprite_glyphs_.clear();
}

void BatchRenderer::end() {
    if (sprite_glyphs_.empty()) {
        return;
    }

    std::ranges::stable_sort(sprite_glyphs_,
                             [](const auto& a, const auto& b) { return a->layer_ < b->layer_; });

    create_batches();
}

void BatchRenderer::add_sprite(const glm::vec4& dest_rect, const glm::vec4& uv_rect, int layer,
                               GLuint texture_id, const Color& color, glm::mat4 model) {
    sprite_glyphs_.emplace_back(std::make_unique<SpriteGlyph>(SpriteGlyph{
        .top_left_ =
            Vertex{.position_ = model * glm::vec4{dest_rect.x, dest_rect.y + dest_rect.w, 0.F, 1.F},
                   .uvs_ = UV{.u_ = uv_rect.x, .v_ = uv_rect.y + uv_rect.w},
                   .color_ = color},
        .top_right_ = Vertex{.position_ = model * glm::vec4{dest_rect.x + dest_rect.z,
                                                            dest_rect.y + dest_rect.w, 0.F, 1.F},
                             .uvs_ = UV{.u_ = uv_rect.x + uv_rect.z, .v_ = uv_rect.y + uv_rect.w},
                             .color_ = color},
        .bottom_left_ = Vertex{.position_ = model * glm::vec4{dest_rect.x, dest_rect.y, 0.F, 1.F},
                               .uvs_ = UV{.u_ = uv_rect.x, .v_ = uv_rect.y},
                               .color_ = color},
        .bottom_right_ =
            Vertex{.position_ = model * glm::vec4{dest_rect.x + dest_rect.z, dest_rect.y, 0.F, 1.F},
                   .uvs_ = UV{.u_ = uv_rect.x + uv_rect.z, .v_ = uv_rect.y},
                   .color_ = color},
        .layer_ = layer,
        .texture_id_ = texture_id}));
}

void BatchRenderer::render() {
    if (batches_.empty()) {
        return;
    }

    glBindVertexArray(vao_);
    for (const auto& batch : batches_) {
        glBindTexture(GL_TEXTURE_2D, batch->texture_id_);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(batch->num_indices_), GL_UNSIGNED_INT,
                       reinterpret_cast<void*>(sizeof(GLuint) * batch->offset_));
    }
}

void BatchRenderer::create_batches() {
    std::vector<Vertex> vertices;
    vertices.resize(sprite_glyphs_.size() * NUM_SPRITE_VERTICES);

    std::size_t current_index{0};
    int current_sprite{0};
    GLuint offset{0};
    GLuint prev_texture_id{0};

    for (const auto& sprite : sprite_glyphs_) {
        if (current_sprite == 0) {
            batches_.emplace_back(std::make_unique<Batch>(Batch{.num_indices_ = NUM_SPRITE_INDICES,
                                                                .texture_id_ = sprite->texture_id_,
                                                                .offset_ = offset}));
        } else if (sprite->texture_id_ != prev_texture_id) {
            batches_.emplace_back(std::make_unique<Batch>(Batch{.num_indices_ = NUM_SPRITE_INDICES,
                                                                .texture_id_ = sprite->texture_id_,
                                                                .offset_ = offset}));
        } else {
            batches_.back()->num_indices_ += NUM_SPRITE_INDICES;
        }

        // Keep winding consistent for both triangles with index pattern {0,1,2, 2,3,0}.
        vertices[current_index++] = sprite->top_left_;
        vertices[current_index++] = sprite->top_right_;
        vertices[current_index++] = sprite->bottom_right_;
        vertices[current_index++] = sprite->bottom_left_;

        prev_texture_id = sprite->texture_id_;
        offset += NUM_SPRITE_INDICES;
        current_sprite++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                    vertices.data());
}

void BatchRenderer::init() {
    sprite_glyphs_.reserve(MAX_SPRITES);
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

    GLuint offset{0};
    GLuint indices[NUM_SPRITE_INDICES]{0, 1, 2, 2, 3, 0};
    auto indices_arr = std::make_unique<GLuint[]>(MAX_INDICES);
    for (std::size_t i = 0; i < MAX_INDICES; i += NUM_SPRITE_INDICES) {
        for (std::size_t j = 0; j < NUM_SPRITE_INDICES; j++) {
            indices_arr[i + j] = indices[j] + offset;
        }
        offset += NUM_SPRITE_VERTICES;
    }

    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * MAX_INDICES, indices_arr.get(),
                 GL_STATIC_DRAW);
}
