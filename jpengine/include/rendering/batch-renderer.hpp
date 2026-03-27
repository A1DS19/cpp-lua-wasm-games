#pragma once

#include "vertex.hpp"
#ifdef __APPLE__

    #include <OpenGL/gl3.h>
#else
    #include <GLES3/gl3.h>
#endif

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float4.hpp>
#include <memory>
#include <vector>

namespace jpengine {
struct Batch {
    GLuint num_indices_{0};
    GLuint texture_id_{0};
    GLuint offset_{0};
};

struct SpriteGlyph {
    Vertex top_left_;
    Vertex top_right_;
    Vertex bottom_left_;
    Vertex bottom_right_;
    int layer_;
    GLuint texture_id_{0};
};

class BatchRenderer {
public:
    BatchRenderer();
    ~BatchRenderer();
    void begin();
    void end();
    void add_sprite(const glm::vec4& dest_rect, const glm::vec4& uv_rect, int layer,
                    GLuint texture_id, const Color& color, glm::mat4 model = glm::mat4{1.F});
    void render();

private:
    void create_batches();
    void init();
    GLuint vao_;
    GLuint vbo_;
    GLuint ibo_;

    std::vector<std::unique_ptr<SpriteGlyph>> sprite_glyphs_;
    std::vector<std::unique_ptr<Batch>> batches_;
};

} // namespace jpengine
