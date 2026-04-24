#pragma once

#include "engine/src/scene/component.hpp"
#include "glm/ext/matrix_float4x4.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace engine {

class Scene;

class GameObject {

public:
    virtual ~GameObject() = default;
    virtual void update(float deltatime);
    [[nodiscard]] const std::string& get_name() const noexcept { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }
    [[nodiscard]] GameObject* get_parent() { return parent_; }
    [[nodiscard]] bool get_is_alive() const noexcept { return is_alive_; }
    void mark_for_destroy();
    [[nodiscard]] glm::vec3& get_position() { return position_; }
    [[nodiscard]] glm::vec3& get_rotation() { return rotation_; }
    [[nodiscard]] glm::vec3& get_scale() { return scale_; }
    void set_position(glm::vec3 position) { position_ = position; }
    void set_rotation(glm::vec3 rotation) { rotation_ = rotation; }
    void set_scale(glm::vec3 scale) { scale_ = scale; }
    [[nodiscard]] glm::mat4 get_local_transform() const;
    [[nodiscard]] glm::mat4 get_world_transform() const;
    void add_component(Component* component) {
        components_.emplace_back(component);
        component->owner_ = this;
    }

protected:
    GameObject() = default;

private:
    std::string name_;
    GameObject* parent_ = nullptr;
    std::vector<std::unique_ptr<GameObject>> children_;
    std::vector<std::unique_ptr<Component>> components_;
    bool is_alive_ = true;
    glm::vec3 position_ = glm::vec3(0.0F);
    glm::vec3 rotation_ = glm::vec3(0.0F);
    glm::vec3 scale_ = glm::vec3(1.0F);

    friend class Scene;
};

} // namespace engine
