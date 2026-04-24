#include "engine/src/render/render-queue.hpp"

#include "engine/src/graphics/graphics-api.hpp"

namespace engine {

void RenderQueue::submit(const RenderCommand& command) {
    commands_.push_back(command);
}

void RenderQueue::draw(class GraphicsApi& graphics_api) {
    for (auto& command : commands_) {
        graphics_api.bind_material(command.material_);
        graphics_api.bind_mesh(command.mesh_);
        graphics_api.draw_mesh(command.mesh_);
    }

    commands_.clear();
}

} // namespace engine
