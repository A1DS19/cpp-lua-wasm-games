#include "engine/src/scene/components/mesh-component.hpp"

#include "engine/src/engine.hpp"
#include "engine/src/render/render-queue.hpp"
#include "engine/src/scene/game-object.hpp"

namespace engine {

MeshComponent::MeshComponent(const std::shared_ptr<Material>& material,
                             const std::shared_ptr<Mesh>& mesh)
    : material_{material}, mesh_{mesh} {}

void MeshComponent::update(float deltatime) {
    if (!material_ || !mesh_) {
        return;
    }

    RenderCommand command;
    command.material_ = material_.get();
    command.mesh_ = mesh_.get();
    command.model_matrix_ = get_owner()->get_world_transform();

    auto& render_queue = Engine::get_instance().get_render_queue();
    render_queue.submit(command);
}

} // namespace engine
