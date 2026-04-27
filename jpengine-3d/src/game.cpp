#include "game.hpp"

#include "engine/src/engine.hpp"
#include "engine/src/scene/components/camera-component.hpp"
#include "test-obj.hpp"

#include <glm/glm.hpp>

bool Game::init() {
    scene_ = new engine::Scene();

    auto camera = scene_->create_object("camera");
    camera->add_component(new engine::CameraComponent());
    camera->set_position(glm::vec3(0.F, 0.F, 2.F));

    scene_->set_main_camera(camera);

    scene_->create_object<TestObject>("test-object");
    engine::Engine::get_instance().set_scene(scene_);
    return true;
}

void Game::update(float deltatime) {
    scene_->update(deltatime);
}

void Game::destroy() {}
