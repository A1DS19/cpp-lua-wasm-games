#include "engine/src/scene/components/camera-component.hpp"

#include "engine/src/scene/game-object.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/trigonometric.hpp>

namespace engine {
void CameraComponent::update(float deltatime) {}

[[nodiscard]] glm::mat4 CameraComponent::get_view_matrix() const {
    return glm::inverse(owner_->get_world_transform());
}

[[nodiscard]] glm::mat4 CameraComponent::get_projection_matrix(float aspect_ratio) const {
    return glm::perspective(glm::radians(fov_), aspect_ratio, near_plane_, far_plane_);
}

} // namespace engine
