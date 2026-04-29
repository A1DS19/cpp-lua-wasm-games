#pragma once
#include "engine/src/scene/component.hpp"

#include <glm/detail/qualifier.hpp>
#include <glm/ext/vector_float3.hpp>

namespace engine {

class LightComponent : public Component {
    COMPONENT(LightComponent)

public:
    void update(float deltatime) override;
    void set_color(glm::vec3 color) { color_ = color; }
    [[nodiscard]] glm::vec3& get_color() { return color_; }

private:
    glm::vec3 color_ = glm::vec3(1.0F);
};

} // namespace engine
