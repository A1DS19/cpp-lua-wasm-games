#include "engine/src/scene/game-object.hpp"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace engine {

void GameObject::update(float deltatime) {
    for (auto& component : components_) {
        component->update(deltatime);
    }

    for (auto it = children_.begin(); it != children_.end();) {
        if ((*it)->is_alive_) {
            (*it)->update(deltatime);
            ++it;
        } else {
            it = children_.erase(it);
        }
    }
}

void GameObject::mark_for_destroy() {
    is_alive_ = false;
}

[[nodiscard]] glm::mat4 GameObject::get_local_transform() const {
    auto mat = glm::mat4(1.0F);

    // translation
    mat = glm::translate(mat, position_);

    // rotation
    mat = glm::rotate(mat, rotation_.x, glm::vec3(1.0F, 0.0F, 0.0F)); // x axis
    mat = glm::rotate(mat, rotation_.r, glm::vec3(0.0F, 1.0F, 0.0F)); // y axis
    mat = glm::rotate(mat, rotation_.z, glm::vec3(0.0F, 0.0F, 1.0F)); // z axis

    // scale
    mat = glm::scale(mat, scale_);

    return mat;
}

[[nodiscard]] glm::mat4 GameObject::get_world_transform() const {
    if (static_cast<bool>(parent_)) {
        return parent_->get_world_transform() * get_local_transform();
    }

    return get_local_transform();
}

} // namespace engine
