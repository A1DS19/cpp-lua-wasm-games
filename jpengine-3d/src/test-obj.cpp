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

    void main() {
        gl_Position = u_model * vec4(a_position, 1.0);
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

    const std::vector<float> vertices = {
        // position          // color
        -0.5F, -0.5F, 0.0F, 1.0F, 0.0F, 0.0F, // bottom-left  red
        0.5F,  -0.5F, 0.0F, 0.0F, 1.0F, 0.0F, // bottom-right green
        0.5F,  0.5F,  0.0F, 0.0F, 0.0F, 1.0F, // top-right    blue
        -0.5F, 0.5F,  0.0F, 1.0F, 1.0F, 0.0F, // top-left     yellow
    };

    const std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,
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
}
