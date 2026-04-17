#include "rendering/shape-batch-renderer.hpp"

#include "rendering/batch-renderer.hpp"
#include "rendering/vertex.hpp"

#include <cmath>
#include <cstddef>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/vector_float2.hpp>
#include <iterator>
#include <sol/types.hpp>

namespace jpengine {

constexpr float PI = 3.1415925f;
constexpr float TWO_PI = PI * 2.F;
constexpr int MAX_SHAPES = 10000;

ShapeRenderer::ShapeRenderer()
    : vao_{0}, vbo_{0}, ebo_{0}, line_vao_{0}, line_vbo_{0}, index_count_{0} {
    initialize();
}

ShapeRenderer::~ShapeRenderer() {
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteVertexArrays(1, &vao_);

    glDeleteBuffers(1, &line_vbo_);
    glDeleteVertexArrays(1, &line_vao_);
}

void ShapeRenderer::begin() {
    vertices_.clear();
    line_vertices_.clear();
    indices_.clear();
    index_count_ = 0;
}

void ShapeRenderer::end() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_.size() * sizeof(ShapeVertex), vertices_.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices_.size() * sizeof(unsigned int),
                    indices_.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, line_vertices_.size() * sizeof(ShapeVertex),
                    line_vertices_.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ShapeRenderer::render() {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(line_vao_);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(line_vertices_.size()));

    glBindVertexArray(0);
}

void ShapeRenderer::add_rectangle(const glm::vec2& position, const glm::vec2& size,
                                  const Color& color) {
    unsigned int i = vertices_.size();

    vertices_.emplace_back(position, color);                          // top left
    vertices_.emplace_back(position + glm::vec2{size.x, 0.F}, color); // top right
    vertices_.emplace_back(position + size, color);                   // botoom right
    vertices_.emplace_back(position + glm::vec2{0, size.y}, color);   // bottom left

    indices_.push_back(i + 0);
    indices_.push_back(i + 1);
    indices_.push_back(i + 2);
    indices_.push_back(i + 2);
    indices_.push_back(i + 3);
    indices_.push_back(i + 0);

    index_count_ += 6;
}

void ShapeRenderer::add_triangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3,
                                 const Color& color) {
    unsigned int i = vertices_.size();

    vertices_.emplace_back(p1, color);
    vertices_.emplace_back(p2, color);
    vertices_.emplace_back(p3, color);

    indices_.push_back(i);
    indices_.push_back(i + 1);
    indices_.push_back(i + 2);

    index_count_ += 3;
}

void ShapeRenderer::add_circle(const glm::vec2& center, float radius, const Color& color,
                               int segments) {
    unsigned int i = vertices_.size();

    vertices_.emplace_back(center, color);
    float fsegments{static_cast<float>(segments)};

    for (int j = 0; j <= segments; j++) {
        float angle = (j / fsegments) * TWO_PI;
        glm::vec2 offset = {cos(angle) * radius, sin(angle) * radius};
        vertices_.emplace_back(center + offset, color);
    }

    for (int j = 0; j <= segments; j++) {
        indices_.push_back(i);
        indices_.push_back(i + j);
        indices_.push_back(i + (j % segments) + 1);

        index_count_ += 3;
    }
}

void ShapeRenderer::add_polygon(const std::vector<glm::vec2>& points, const Color& color) {
    if (points.size() < 3) {
        return;
    }

    unsigned int i = vertices_.size();

    for (const auto& point : points) {
        vertices_.emplace_back(point, color);
    }

    for (size_t j = 1; j < points.size() - 1; j++) {
        indices_.push_back(i);
        indices_.push_back(i + j);
        indices_.push_back(i + j + 1);
        index_count_ += 3;
    }
}

void ShapeRenderer::add_line(const glm::vec2& p1, const glm::vec2& p2, const Color& color) {
    line_vertices_.emplace_back(p1, color);
    line_vertices_.emplace_back(p2, color);
}

void ShapeRenderer::add_wire_rectangle(const glm::vec2& position, const glm::vec2& size,
                                       const Color& color) {
    glm::vec2 p1 = position;
    glm::vec p2 = position + glm::vec2{size.x, 0};
    glm::vec2 p3 = position + size;
    glm::vec2 p4 = position + glm::vec2{0, size.y};

    add_line(p1, p2, color);
    add_line(p2, p3, color);
    add_line(p3, p4, color);
    add_line(p4, p1, color);
}

void ShapeRenderer::add_wire_triangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3,
                                      const Color& color) {
    add_line(p1, p2, color);
    add_line(p2, p3, color);
    add_line(p3, p1, color);
}

void ShapeRenderer::add_wire_circle(const glm::vec2& center, float radius, const Color& color,
                                    int segments) {
    float angle_step = TWO_PI / segments;
    for (int i = 0; i < segments; i++) {
        float a0 = angle_step * i;
        float a1 = angle_step * (i + 1);

        glm::vec2 p0 = center + radius * glm::vec2{std::cos(a0), std::sin(a0)};
        glm::vec2 p1 = center + radius * glm::vec2{std::cos(a1), std::sin(a1)};

        add_line(p0, p1, color);
    }
}

void ShapeRenderer::add_wire_polygon(const std::vector<glm::vec2>& points, const Color& color) {
    size_t n = points.size();
    for (size_t i = 0; i < n; ++i) {
        add_line(points[i], points[(i + 1) % n], color);
    }
}

void ShapeRenderer::initialize() {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, MAX_SHAPES * sizeof(ShapeVertex), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_SHAPES * sizeof(unsigned int), nullptr,
                 GL_DYNAMIC_DRAW);

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),
                          (void*)offsetof(ShapeVertex, position_));
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ShapeVertex),
                          (void*)offsetof(ShapeVertex, color_));
    glEnableVertexAttribArray(1);

    // ADDED!!
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &line_vao_);
    glGenBuffers(1, &line_vbo_);

    glBindVertexArray(line_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);
    glBufferData(GL_ARRAY_BUFFER, MAX_SHAPES * sizeof(ShapeVertex), nullptr, GL_DYNAMIC_DRAW);

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),
                          (void*)offsetof(ShapeVertex, position_));
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ShapeVertex),
                          (void*)offsetof(ShapeVertex, color_));
    glEnableVertexAttribArray(1);

    // ADDED!!
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);
}

} // namespace jpengine
