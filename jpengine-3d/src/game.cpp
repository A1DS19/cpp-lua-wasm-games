#include "game.hpp"

#include "engine/src/engine.hpp"
#include "engine/src/scene/components/camera-component.hpp"
#include "engine/src/scene/components/light-component.hpp"
#include "engine/src/scene/components/player-controller-component.hpp"
#include "engine/src/scene/game-object.hpp"
#include "engine/src/scene/scene.hpp"
#include "test-obj.hpp"

#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <glm/vec3.hpp>

namespace {
constexpr glm::vec3 CAMERA_START_POSITION = glm::vec3(0.0F, 0.0F, 5.0F);
constexpr glm::vec3 SUN_POSITION = glm::vec3(2.0F, 3.0F, 4.0F);
constexpr glm::vec3 CUBE_POSITION = glm::vec3(-2.0F, 0.0F, 0.0F);
constexpr glm::vec3 SUZANNE_POSITION = glm::vec3(2.0F, 0.0F, 0.0F);
constexpr glm::vec3 GUN_OFFSET = glm::vec3(0.3F, -0.3F, -0.6F);
constexpr glm::vec3 GUN_SCALE = glm::vec3(-0.3F, 0.3F, 0.3F);
} // namespace

bool Game::init() {
    scene_ = new engine::Scene();

    // Register the scene with the engine FIRST so anything that calls
    // Engine::get_current_scene() during construction (notably load_gltf)
    // sees the active scene.
    engine::Engine::get_instance().set_scene(scene_);

    // Camera
    auto* camera = scene_->create_object("camera");
    camera->add_component(new engine::CameraComponent());
    camera->add_component(new engine::PlayerControllerComponent());
    camera->set_position(CAMERA_START_POSITION);
    scene_->set_main_camera(camera);

    // Sun light — placed up-and-forward of the scene so meshes are lit from
    // above-right. A LightComponent is just an emitter; it doesn't render
    // anything itself.
    auto* sun = scene_->create_object("sun");
    sun->add_component(new engine::LightComponent());
    sun->set_position(SUN_POSITION);

    // Brick cube on the left.
    auto* cube = scene_->create_object<TestObject>("cube");
    cube->set_position(CUBE_POSITION);

    // Suzanne loaded automatically from glTF on the right — material/texture
    // extracted from the file, transforms inherited from glTF nodes.
    auto* suzanne = engine::GameObject::load_gltf("models/suzanne/Suzanne.gltf");
    if (suzanne != nullptr) {
        suzanne->set_position(SUZANNE_POSITION);
    }

    auto* gun = engine::GameObject::load_gltf("models/sten_gun/scene.gltf");
    if (gun != nullptr) {
        gun->set_parent(camera);
        gun->set_position(GUN_OFFSET);
        gun->set_scale(GUN_SCALE);
    }

    return true;
}

void Game::update(float deltatime) {
    scene_->update(deltatime);
}

void Game::destroy() {}
