#pragma once

#include <cstdint>
#include <vector>
namespace engine {

struct VertexElement {
    uint32_t index_;  // attribute location
    uint32_t size_;   // number of components
    uint32_t type_;   // data type (e.g GL_FLOAT)
    uint32_t offset_; // bytes offset from the start of vertex

    // Attribute slot numbers — must match `layout(location = N)` in the shaders.
    static constexpr int position_index_ = 0;
    static constexpr int color_index_ = 1;
    static constexpr int uv_index_ = 2;
    static constexpr int normal_index_ = 3;
};

struct VertexLayout {
    std::vector<VertexElement> elements_;
    uint32_t stride_ = 0; // total size of single vertex
};

} // namespace engine
