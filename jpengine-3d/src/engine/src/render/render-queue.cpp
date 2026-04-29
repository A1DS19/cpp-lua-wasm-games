#include "engine/src/render/render-queue.hpp"

#include "engine/common.hpp"
#include "engine/src/graphics/graphics-api.hpp"
#include "engine/src/graphics/shader-program.hpp"
#include "engine/src/render/material.hpp"

namespace engine {

void RenderQueue::submit(const RenderCommand& command) {
    commands_.push_back(command);
}

void RenderQueue::draw(class GraphicsApi& graphics_api, const CameraData* camera_data,
                       std::vector<LightData> lights) {
    for (auto& command : commands_) {
        graphics_api.bind_material(command.material_);
        command.material_->get_shader_program()->set_uniform("u_model", command.model_matrix_);
        graphics_api.bind_mesh(command.mesh_);
        graphics_api.draw_mesh(command.mesh_);
        auto shader_pogram = command.material_->get_shader_program();
        shader_pogram->set_uniform("u_view", camera_data->view_matrix_);
        shader_pogram->set_uniform("u_projection", camera_data->projection_matrix_);
        shader_pogram->set_uniform("u_camera_pos", camera_data->position_);
        if (!lights.empty()) {
            auto& light = lights[0];
            shader_pogram->set_uniform("u_light.color", light.color_);
            shader_pogram->set_uniform("u_light.position", light.position_);
        }
    }

    commands_.clear();
}

} // namespace engine
