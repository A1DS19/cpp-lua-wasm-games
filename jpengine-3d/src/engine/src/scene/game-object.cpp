#include "engine/src/scene/game-object.hpp"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

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

    // rotation, converts quat to rotation matrix
    mat = mat * glm::mat4_cast(rotation_);

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

[[nodiscard]] glm::vec3 GameObject::get_world_position() const {
    glm::vec4 hom = get_world_transform() * glm::vec4(0.0F, 0.0F, 0.0F, 1.0F);
    return glm::vec3(hom) / hom.w;
}

} // namespace engine
