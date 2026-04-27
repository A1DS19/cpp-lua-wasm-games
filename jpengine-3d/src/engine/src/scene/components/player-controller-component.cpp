#include "engine/src/scene/components/player-controller-component.hpp"

#include "GLFW/glfw3.h"
#include "engine/src/engine.hpp"
#include "engine/src/input/input-manager.hpp"

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>

namespace engine {
void PlayerControllerComponent::update(float deltatime) {
    auto& input_manager = Engine::get_instance().get_input_manager();
    auto rotation = owner_->get_rotation();

    if (input_manager.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        const auto& old_pos = input_manager.get_mouse_position_old();
        const auto& current_pos = input_manager.get_mouse_position_current();

        float delta_x = current_pos.x - old_pos.x;
        float delta_y = current_pos.y - old_pos.y;

        // rotation around Y axis
        rotation.y -= delta_x * sensitivity_ * deltatime;

        // rotation around X axis
        rotation.x -= delta_y * sensitivity_ * deltatime;

        owner_->set_rotation(rotation);
    }

    glm::mat4 rot_mat(1.0F);
    rot_mat = glm::rotate(rot_mat, rotation.x, glm::vec3(1.0F, 0.0F, 0.0F)); // x axis
    rot_mat = glm::rotate(rot_mat, rotation.y, glm::vec3(0.0F, 1.0F, 0.0F)); // y axis
    rot_mat = glm::rotate(rot_mat, rotation.z, glm::vec3(0.0F, 0.0F, 1.0F)); // z axis

    glm::vec3 front = glm::normalize(glm::vec3(rot_mat * glm::vec4(0.0F, 0.0F, -1.0F, 0.0F)));
    glm::vec3 right = glm::normalize(glm::vec3(rot_mat * glm::vec4(1.0F, 0.0F, 0.0F, 0.0F)));

    auto position = owner_->get_position();

    if (input_manager.is_key_pressed(GLFW_KEY_W)) {
        position += front * move_speed_ * deltatime;
    }
    if (input_manager.is_key_pressed(GLFW_KEY_A)) {
        position -= right * move_speed_ * deltatime;
    }
    if (input_manager.is_key_pressed(GLFW_KEY_S)) {
        position -= front * move_speed_ * deltatime;
    }
    if (input_manager.is_key_pressed(GLFW_KEY_D)) {
        position += right * move_speed_ * deltatime;
    }

    owner_->set_position(position);
}

} // namespace engine
