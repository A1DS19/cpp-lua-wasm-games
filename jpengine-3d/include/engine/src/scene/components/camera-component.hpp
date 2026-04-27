#pragma once
#include "engine/src/scene/component.hpp"

#include <glm/ext/matrix_float4x4.hpp>

namespace engine {

class CameraComponent : public Component {
    COMPONENT(CameraComponent);

public:
    void update(float deltatime) override;
    [[nodiscard]] glm::mat4 get_view_matrix() const;
    [[nodiscard]] glm::mat4 get_projection_matrix(float aspect_ratio) const;

private:
    float fov_ = 60.F;
    float near_plane_ = 0.1F;
    float far_plane_ = 1000.F;
};

} // namespace engine
