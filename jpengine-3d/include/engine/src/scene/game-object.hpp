#pragma once

#include "engine/src/scene/component.hpp"

#include <cstddef>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
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
    // Reparents this object inside its scene. Returns false if the object has
    // no scene yet, or if the move would create a cycle.
    bool set_parent(GameObject* parent);

    [[nodiscard]] Scene* get_scene() { return scene_; }

    [[nodiscard]] bool get_is_alive() const noexcept { return is_alive_; }
    void mark_for_destroy();

    // Active = "ticked by update each frame." An inactive object stays in the
    // scene tree (still owned, still rendered if it has a MeshComponent that
    // was already submitted) but skips its own update + its children's update.
    void set_active(bool active) noexcept { active_ = active; }
    [[nodiscard]] bool is_active() const noexcept { return active_; }
    [[nodiscard]] glm::vec3& get_position() { return position_; }
    [[nodiscard]] glm::quat& get_rotation() { return rotation_; }
    [[nodiscard]] glm::vec3& get_scale() { return scale_; }
    void set_position(glm::vec3 position) { position_ = position; }
    void set_rotation(glm::quat rotation) { rotation_ = rotation; }
    void set_scale(glm::vec3 scale) { scale_ = scale; }
    [[nodiscard]] glm::mat4 get_local_transform() const;
    [[nodiscard]] glm::mat4 get_world_transform() const;
    void add_component(Component* component) {
        components_.emplace_back(component);
        component->owner_ = this;
    }

    template <std::derived_from<Component> T>
    T* get_component() {
        std::size_t type_id = Component::static_type_id<T>();
        for (auto& component : components_) {
            if (component->get_type_id() == type_id) {
                return static_cast<T*>(component.get());
            }
        }

        return nullptr;
    }
    [[nodiscard]] glm::vec3 get_world_position() const;

    // Recursive depth-first search through children. Returns the first
    // descendant whose name matches, or nullptr.
    [[nodiscard]] GameObject* find_child_by_name(const std::string& name);

    static GameObject* load_gltf(const std::string& path);

protected:
    GameObject() = default;

private:
    std::string name_;
    GameObject* parent_ = nullptr;
    Scene* scene_ = nullptr;
    std::vector<std::unique_ptr<GameObject>> children_;
    std::vector<std::unique_ptr<Component>> components_;
    bool is_alive_ = true;
    bool active_ = true;
    glm::vec3 position_ = glm::vec3(0.0F);
    glm::quat rotation_ = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
    glm::vec3 scale_ = glm::vec3(1.0F);

    friend class Scene;
};

} // namespace engine
