#pragma once

#include <cstdint>
#include <vector>
namespace engine {

struct VertexElement {
    uint32_t index_;  // attribute location
    uint32_t size_;   // number of components
    uint32_t type_;   // data type (e.g GL_FLOAT)
    uint32_t offset_; // bytes offset from the start of vertex
};

struct VertexLayout {
    std::vector<VertexElement> elements_;
    uint32_t stride_ = 0; // total size of single vertex
};

} // namespace engine
