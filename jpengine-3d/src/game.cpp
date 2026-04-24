#include "game.hpp"

#include "GLFW/glfw3.h"
#include "engine/src/engine.hpp"
#include "engine/src/render/material.hpp"
#include "engine/src/render/render-queue.hpp"

#include <GL/glew.h>
#include <iostream>

bool Game::init() {
    const std::string vertex_source = R"(
    #version 330 core
    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec3 a_color;
    out vec3 v_color;
    void main() {
        gl_Position = vec4(a_position, 1.0);
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

    pshader_program_ = engine::Engine::get_instance().get_graphics_api().create_shader_program(
        vertex_source, fragment_source);

    material_.set_shader_program(pshader_program_);

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

    mesh_ = std::make_unique<engine::Mesh>(layout, vertices, indices);

    return pshader_program_ != nullptr;
}

void Game::update(float) {
    auto& input = engine::Engine::get_instance().get_input_manager();
    if (input.is_key_pressed(GLFW_KEY_A)) {
        std::cout << "key a pressed\n";
    }

    engine::RenderCommand command;
    command.material_ = &material_;
    command.mesh_ = mesh_.get();

    engine::Engine::get_instance().get_render_queue().submit(command);
}

void Game::destroy() {}
