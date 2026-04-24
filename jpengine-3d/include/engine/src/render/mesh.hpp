#pragma once

#include "engine/src/graphics/vertex-layout.hpp"
#include "glew/include/GL/glew.h"

#include <cstddef>
#include <cstdint>

namespace engine {

class Mesh {
public:
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(const VertexLayout& layout, const std::vector<float> vertices,
         const std::vector<uint32_t> indices);
    Mesh(const VertexLayout& layout, const std::vector<float> vertices);

    void bind();
    void draw();

private:
    VertexLayout vertex_layout_;
    size_t vertex_count_ = 0;
    size_t index_count_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLuint vao_ = 0;
};

} // namespace engine
