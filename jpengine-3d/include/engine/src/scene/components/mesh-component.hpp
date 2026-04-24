#pragma once

#include "engine/src/scene/component.hpp"

#include <memory>

namespace engine {

class Material;
class Mesh;
class MeshComponent : public Component {
public:
    MeshComponent(const std::shared_ptr<Material>& material, const std::shared_ptr<Mesh>& mesh);
    void update(float deltatime) override;

private:
    std::shared_ptr<Material> material_;
    std::shared_ptr<Mesh> mesh_;
};

} // namespace engine
