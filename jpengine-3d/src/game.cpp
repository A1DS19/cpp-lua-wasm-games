#include "game.hpp"

#include "engine/src/engine.hpp"
#include "engine/src/scene/components/camera-component.hpp"
#include "engine/src/scene/components/player-controller-component.hpp"
#include "suzanne-obj.hpp"
#include "test-obj.hpp"

#include <glm/glm.hpp>

bool Game::init() {
    scene_ = new engine::Scene();

    // Camera
    auto* camera = scene_->create_object("camera");
    camera->add_component(new engine::CameraComponent());
    camera->add_component(new engine::PlayerControllerComponent());
    camera->set_position(glm::vec3(0.0F, 0.0F, 4.0F));
    scene_->set_main_camera(camera);

    // Brick cube on the left
    auto* cube = scene_->create_object<TestObject>("cube");
    cube->set_position(glm::vec3(-1.5F, 0.0F, 0.0F));

    // Suzanne (glTF) on the right
    auto* suzanne = scene_->create_object<SuzanneObject>("suzanne");
    suzanne->set_position(glm::vec3(1.5F, 0.0F, 0.0F));

    engine::Engine::get_instance().set_scene(scene_);
    return true;
}

void Game::update(float deltatime) {
    scene_->update(deltatime);
}

void Game::destroy() {}
