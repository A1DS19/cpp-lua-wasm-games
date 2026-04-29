#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

namespace engine {

struct CameraData {
    glm::mat4 view_matrix_;
    glm::mat4 projection_matrix_;
    glm::vec3 position_;
};

struct LightData {
    glm::vec3 color_;
    glm::vec3 position_;
};

} // namespace engine
