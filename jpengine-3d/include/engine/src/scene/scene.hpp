#pragma once

#include "engine/common.hpp"
#include "engine/src/scene/game-object.hpp"

#include <concepts>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace engine {

class Scene {

public:
    void update(float deltatime);
    void clear();

    GameObject* create_object(std::string name, GameObject* parent = nullptr);

    // Template definition must live in the header — otherwise specializations
    // for types `T` defined outside this TU can't be instantiated at link time.
    template <std::derived_from<GameObject> T>
    T* create_object(std::string name, GameObject* parent = nullptr) {
        auto* object = new T();
        object->set_name(std::move(name));
        object->scene_ = this;
        set_parent(object, parent);
        return object;
    }

    bool set_parent(GameObject* object, GameObject* parent);
    void set_main_camera(GameObject* camera) { main_camera_ = camera; }
    [[nodiscard]] GameObject* get_main_camera() const { return main_camera_; }
    std::vector<LightData> collect_light();

private:
    void collect_light_recursive(GameObject* obj, std::vector<LightData>& out);
    std::vector<std::unique_ptr<GameObject>> objects_;
    GameObject* main_camera_ = nullptr;
};

} // namespace engine
