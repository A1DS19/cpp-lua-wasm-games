#include "test-obj.hpp"

#include "GLFW/glfw3.h"
#include "engine/src/graphics/shader-program.hpp"
#include "engine/src/render/material.hpp"
#include "engine/src/scene/components/mesh-component.hpp"
#include "engine/src/scene/game-object.hpp"

#include <memory>

TestObject::TestObject() {

    const std::string vertex_source = R"(
    #version 330 core
    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec3 a_color;
    out vec3 v_color;

    uniform mat4 u_model;
    uniform mat4 u_view;
    uniform mat4 u_projection;

    void main() {
        gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
        v_color = a_color;
    }
)";

    const std::string fragment_source = R"(
    #version 330 core
    in vec3 v_color;
    out vec4 frag_color;
    void main() {
        frag_color = vec4(v_color, 1.0);
    }
)";

    auto pshader_program = engine::Engine::get_instance().get_graphics_api().create_shader_program(
        vertex_source, fragment_source);
    auto material = std::make_shared<engine::Material>();
    material->set_shader_program(pshader_program);

    // 8 unique corners. Color = each corner's position mapped into [0, 1] for RGB,
    // so every corner has a distinct color and adjacent faces interpolate smoothly.
    const std::vector<float> vertices = {
        //  x      y      z      r     g     b
        -0.5F, -0.5F, -0.5F, 0.0F, 0.0F, 0.0F, // 0: back-bottom-left   black
        0.5F,  -0.5F, -0.5F, 1.0F, 0.0F, 0.0F, // 1: back-bottom-right  red
        0.5F,  0.5F,  -0.5F, 1.0F, 1.0F, 0.0F, // 2: back-top-right     yellow
        -0.5F, 0.5F,  -0.5F, 0.0F, 1.0F, 0.0F, // 3: back-top-left      green
        -0.5F, -0.5F, 0.5F,  0.0F, 0.0F, 1.0F, // 4: front-bottom-left  blue
        0.5F,  -0.5F, 0.5F,  1.0F, 0.0F, 1.0F, // 5: front-bottom-right magenta
        0.5F,  0.5F,  0.5F,  1.0F, 1.0F, 1.0F, // 6: front-top-right    white
        -0.5F, 0.5F,  0.5F,  0.0F, 1.0F, 1.0F, // 7: front-top-left     cyan
    };

    // 6 faces × 2 triangles each. Wound counter-clockwise as seen from outside,
    // so default GL face culling (front = CCW) keeps them all visible.
    const std::vector<uint32_t> indices = {
        // Front (+z)
        4,
        5,
        6,
        4,
        6,
        7,
        // Back (-z)
        1,
        0,
        3,
        1,
        3,
        2,
        // Right (+x)
        5,
        1,
        2,
        5,
        2,
        6,
        // Left (-x)
        0,
        4,
        7,
        0,
        7,
        3,
        // Top (+y)
        7,
        6,
        2,
        7,
        2,
        3,
        // Bottom (-y)
        0,
        1,
        5,
        0,
        5,
        4,
    };

    engine::VertexLayout layout{
        .elements_ =
            {
                {.index_ = 0, .size_ = 3, .type_ = GL_FLOAT, .offset_ = 0},
                {.index_ = 1,
                 .size_ = 3,
                 .type_ = GL_FLOAT,
                 .offset_ = static_cast<uint32_t>(3 * sizeof(float))},
            },
        .stride_ = static_cast<uint32_t>(6 * sizeof(float)),
    };

    auto mesh = std::make_shared<engine::Mesh>(layout, vertices, indices);

    add_component(new engine::MeshComponent(material, mesh));
}

void TestObject::update(float deltatime) {

    engine::GameObject::update(deltatime);

    // Spin the cube so all 6 faces become visible.
    auto& rotation = get_rotation();
    rotation.y += deltatime;          // ~1 rad/s around Y
    rotation.x += deltatime * 0.5F;   // ~0.5 rad/s around X (slight tilt)

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
