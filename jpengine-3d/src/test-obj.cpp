#include "test-obj.hpp"

#include "engine/src/graphics/shader-program.hpp"
#include "engine/src/render/material.hpp"
#include "engine/src/scene/components/mesh-component.hpp"
#include "engine/src/scene/game-object.hpp"

#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <stb/stb_image.h>

TestObject::TestObject() {

    auto material = engine::Material::load("materials/brick.mat");

    // 24 vertices (4 per face) so each face can have its own UVs.
    // Each vertex carries position (xyz), color (rgb) and a texture coord (uv).
    // Per-face vertices are listed BL, BR, TR, TL — CCW seen from outside the cube,
    // so default GL face culling (front = CCW) keeps them all visible.
    const std::vector<float> vertices = {
        //   x      y      z      r     g     b      u     v
        // ---- Front face (+z) ----
        -0.5F,
        -0.5F,
        0.5F,
        0.0F,
        0.0F,
        1.0F,
        0.0F,
        0.0F, // BL  blue
        0.5F,
        -0.5F,
        0.5F,
        1.0F,
        0.0F,
        1.0F,
        1.0F,
        0.0F, // BR  magenta
        0.5F,
        0.5F,
        0.5F,
        1.0F,
        1.0F,
        1.0F,
        1.0F,
        1.0F, // TR  white
        -0.5F,
        0.5F,
        0.5F,
        0.0F,
        1.0F,
        1.0F,
        0.0F,
        1.0F, // TL  cyan

        // ---- Back face (-z) ----
        0.5F,
        -0.5F,
        -0.5F,
        1.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F, // BL  red
        -0.5F,
        -0.5F,
        -0.5F,
        0.0F,
        0.0F,
        0.0F,
        1.0F,
        0.0F, // BR  black
        -0.5F,
        0.5F,
        -0.5F,
        0.0F,
        1.0F,
        0.0F,
        1.0F,
        1.0F, // TR  green
        0.5F,
        0.5F,
        -0.5F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        1.0F, // TL  yellow

        // ---- Right face (+x) ----
        0.5F,
        -0.5F,
        0.5F,
        1.0F,
        0.0F,
        1.0F,
        0.0F,
        0.0F, // BL  magenta
        0.5F,
        -0.5F,
        -0.5F,
        1.0F,
        0.0F,
        0.0F,
        1.0F,
        0.0F, // BR  red
        0.5F,
        0.5F,
        -0.5F,
        1.0F,
        1.0F,
        0.0F,
        1.0F,
        1.0F, // TR  yellow
        0.5F,
        0.5F,
        0.5F,
        1.0F,
        1.0F,
        1.0F,
        0.0F,
        1.0F, // TL  white

        // ---- Left face (-x) ----
        -0.5F,
        -0.5F,
        -0.5F,
        0.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F, // BL  black
        -0.5F,
        -0.5F,
        0.5F,
        0.0F,
        0.0F,
        1.0F,
        1.0F,
        0.0F, // BR  blue
        -0.5F,
        0.5F,
        0.5F,
        0.0F,
        1.0F,
        1.0F,
        1.0F,
        1.0F, // TR  cyan
        -0.5F,
        0.5F,
        -0.5F,
        0.0F,
        1.0F,
        0.0F,
        0.0F,
        1.0F, // TL  green

        // ---- Top face (+y) ----
        -0.5F,
        0.5F,
        0.5F,
        0.0F,
        1.0F,
        1.0F,
        0.0F,
        0.0F, // BL  cyan
        0.5F,
        0.5F,
        0.5F,
        1.0F,
        1.0F,
        1.0F,
        1.0F,
        0.0F, // BR  white
        0.5F,
        0.5F,
        -0.5F,
        1.0F,
        1.0F,
        0.0F,
        1.0F,
        1.0F, // TR  yellow
        -0.5F,
        0.5F,
        -0.5F,
        0.0F,
        1.0F,
        0.0F,
        0.0F,
        1.0F, // TL  green

        // ---- Bottom face (-y) ----
        -0.5F,
        -0.5F,
        -0.5F,
        0.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F, // BL  black
        0.5F,
        -0.5F,
        -0.5F,
        1.0F,
        0.0F,
        0.0F,
        1.0F,
        0.0F, // BR  red
        0.5F,
        -0.5F,
        0.5F,
        1.0F,
        0.0F,
        1.0F,
        1.0F,
        1.0F, // TR  magenta
        -0.5F,
        -0.5F,
        0.5F,
        0.0F,
        0.0F,
        1.0F,
        0.0F,
        1.0F, // TL  blue
    };

    // 6 faces × 2 triangles each. Each face is 4 sequential vertices; for
    // group N (0–5), the two triangles are (4N, 4N+1, 4N+2) and (4N, 4N+2, 4N+3).
    const std::vector<uint32_t> indices = {
        0,  1,  2,  0,  2,  3,  // front
        4,  5,  6,  4,  6,  7,  // back
        8,  9,  10, 8,  10, 11, // right
        12, 13, 14, 12, 14, 15, // left
        16, 17, 18, 16, 18, 19, // top
        20, 21, 22, 20, 22, 23, // bottom
    };

    engine::VertexLayout layout{
        .elements_ =
            {
                // attribute 0: position (xyz), 3 floats, offset 0
                {.index_ = 0, .size_ = 3, .type_ = GL_FLOAT, .offset_ = 0},
                // attribute 1: color (rgb), 3 floats, offset 3*float
                {.index_ = 1,
                 .size_ = 3,
                 .type_ = GL_FLOAT,
                 .offset_ = static_cast<uint32_t>(3 * sizeof(float))},
                // attribute 2: texture coord (uv), 2 floats, offset 6*float
                {.index_ = 2,
                 .size_ = 2,
                 .type_ = GL_FLOAT,
                 .offset_ = static_cast<uint32_t>(6 * sizeof(float))},
            },
        .stride_ = static_cast<uint32_t>(8 * sizeof(float)),
    };

    auto mesh = std::make_shared<engine::Mesh>(layout, vertices, indices);

    add_component(new engine::MeshComponent(material, mesh));
}

void TestObject::update(float deltatime) {

    engine::GameObject::update(deltatime);

    // Spin the cube so all 6 faces become visible.
    // Quaternions don't rotate by adding to .x/.y — multiply by a delta-rotation.
    const auto delta_y = glm::angleAxis(deltatime, glm::vec3(0.0F, 1.0F, 0.0F));
    const auto delta_x = glm::angleAxis(deltatime * 0.5F, glm::vec3(1.0F, 0.0F, 0.0F));
    set_rotation(delta_y * delta_x * get_rotation());

#if 0
    auto& input = engine::Engine::get_instance().get_input_manager();
    auto position = get_position();
    if (input.is_key_pressed(GLFW_KEY_W)) {
        position.y += 0.01F;
    }
    if (input.is_key_pressed(GLFW_KEY_A)) {
        position.x -= 0.01F;
    }
    if (input.is_key_pressed(GLFW_KEY_S)) {
        position.y -= 0.01F;
    }
    if (input.is_key_pressed(GLFW_KEY_D)) {
        position.x += 0.01F;
    }

    set_position(position);
#endif
}
