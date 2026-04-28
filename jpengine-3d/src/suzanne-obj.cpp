#include "suzanne-obj.hpp"

#include "engine/src/render/material.hpp"
#include "engine/src/render/mesh.hpp"
#include "engine/src/scene/components/mesh-component.hpp"

#include <glm/gtc/quaternion.hpp>

SuzanneObject::SuzanneObject() {
    auto mesh = engine::Mesh::load("models/suzanne/Suzanne.gltf");
    auto material = engine::Material::load("materials/suzanne.mat");
    add_component(new engine::MeshComponent(material, mesh));
}

void SuzanneObject::update(float deltatime) {
    engine::GameObject::update(deltatime);

    // Spin slowly on Y so we can see all sides.
    const auto delta = glm::angleAxis(deltatime * 0.5F, glm::vec3(0.0F, 1.0F, 0.0F));
    set_rotation(delta * get_rotation());
}
