#include "engine/src/render/mesh.hpp"

#include "engine/src/engine.hpp"

#include <GL/glew.h>
#include <cstdint>

namespace engine {

void Mesh::bind() {
    glBindVertexArray(vao_);
}

void Mesh::draw() {
    if (index_count_ > 0) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count_), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertex_count_));
    }
}

Mesh::Mesh(const VertexLayout& layout, const std::vector<float> vertices,
           const std::vector<uint32_t> indices) {
    vertex_layout_ = layout;
    auto& graphics_api = engine::Engine::get_instance().get_graphics_api();

    vbo_ = graphics_api.create_vertex_buffer(vertices);
    ebo_ = graphics_api.create_index_buffer(indices);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    for (auto& element : vertex_layout_.elements_) {
        glVertexAttribPointer(element.index_, static_cast<GLint>(element.size_), element.type_,
                              GL_FALSE, static_cast<GLsizei>(vertex_layout_.stride_),
                              reinterpret_cast<void*>(static_cast<uintptr_t>(element.offset_)));
        glEnableVertexAttribArray(element.index_);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    vertex_count_ = (vertices.size() * sizeof(float)) / vertex_layout_.stride_;
    index_count_ = indices.size();
}

Mesh::Mesh(const VertexLayout& layout, const std::vector<float> vertices) {
    vertex_layout_ = layout;
    auto& graphics_api = engine::Engine::get_instance().get_graphics_api();

    vbo_ = graphics_api.create_vertex_buffer(vertices);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    for (auto& element : vertex_layout_.elements_) {
        glVertexAttribPointer(element.index_, static_cast<GLint>(element.size_), element.type_,
                              GL_FALSE, static_cast<GLsizei>(vertex_layout_.stride_),
                              reinterpret_cast<void*>(static_cast<uintptr_t>(element.offset_)));
        glEnableVertexAttribArray(element.index_);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vertex_count_ = (vertices.size() * sizeof(float)) / vertex_layout_.stride_;
}

} // namespace engine
