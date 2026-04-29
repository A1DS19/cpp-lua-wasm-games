#include "engine/src/scene/scene.hpp"

#include "engine/common.hpp"
#include "engine/src/scene/components/light-component.hpp"
#include "engine/src/scene/game-object.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace engine {
void Scene::update(float deltatime) {

    for (auto it = objects_.begin(); it != objects_.end();) {
        if ((*it)->get_is_alive()) {
            (*it)->update(deltatime);
            ++it;
        } else {
            it = objects_.erase(it);
        }
    }
}

void Scene::clear() {
    objects_.clear();
}

GameObject* Scene::create_object(std::string name, GameObject* parent) {
    auto* object = new GameObject();
    object->set_name(std::move(name));
    set_parent(object, parent);
    return object;
}

bool Scene::set_parent(GameObject* object, GameObject* parent) {
    if (object == nullptr) {
        return false;
    }

    // Cycle prevention: if `object` appears in `parent`'s ancestor chain (or
    // `parent` IS `object`), reparenting would create a cycle.
    for (auto* ancestor = parent; ancestor != nullptr; ancestor = ancestor->get_parent()) {
        if (ancestor == object) {
            return false;
        }
    }

    auto* current_parent = object->get_parent();

    // Helper: find the unique_ptr that owns `object` inside a container.
    const auto find_owner = [object](auto& container) {
        return std::find_if(
            container.begin(), container.end(),
            [object](const std::unique_ptr<GameObject>& el) { return el.get() == object; });
    };

    // --- Clearing parent: move `object` to scene root ---
    if (parent == nullptr) {
        if (current_parent != nullptr) {
            // Currently a child of `current_parent` — pluck it out.
            auto it = find_owner(current_parent->children_);
            if (it == current_parent->children_.end()) {
                return false; // tree is inconsistent
            }
            objects_.push_back(std::move(*it));
            current_parent->children_.erase(it);
            object->parent_ = nullptr;
            return true;
        }

        // No current parent: either already at scene root, or just created.
        auto it = find_owner(objects_);
        if (it != objects_.end()) {
            return false; // already at scene root — nothing to do
        }
        // Just created — adopt the raw pointer at scene root.
        objects_.push_back(std::unique_ptr<GameObject>{object});
        object->parent_ = nullptr;
        return true;
    }

    // --- Assigning a new parent ---
    if (current_parent != nullptr) {
        // Move from old parent's children to new parent's children.
        auto it = find_owner(current_parent->children_);
        if (it == current_parent->children_.end()) {
            return false; // tree is inconsistent
        }
        parent->children_.push_back(std::move(*it));
        current_parent->children_.erase(it);
        object->parent_ = parent;
        return true;
    }

    // No current parent: either at scene root, or just created.
    auto it = find_owner(objects_);
    if (it != objects_.end()) {
        // Move from scene root into `parent`'s children.
        parent->children_.push_back(std::move(*it));
        objects_.erase(it);
    } else {
        // Just created — adopt the raw pointer under `parent`.
        parent->children_.push_back(std::unique_ptr<GameObject>{object});
    }
    object->parent_ = parent;
    return true;
}

std::vector<LightData> Scene::collect_light() {
    std::vector<LightData> lights;
    for (auto& obj : objects_) {
        collect_light_recursive(obj.get(), lights);
    }

    return lights;
}

void Scene::collect_light_recursive(GameObject* obj, std::vector<LightData>& out) {
    if (auto light = obj->get_component<LightComponent>()) {
        LightData data;
        data.color_ = light->get_color();
        data.position_ = obj->get_world_position();
        out.push_back(data);
    }

    for (auto& child : obj->children_) {
        collect_light_recursive(child.get(), out);
    }
}

} // namespace engine
