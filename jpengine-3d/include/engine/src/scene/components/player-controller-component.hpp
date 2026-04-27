#pragma once

#include "engine/src/scene/component.hpp"

#include <array>
namespace engine {

class PlayerControllerComponent : public Component {
    COMPONENT(PlayerControllerComponent)
public:
    void update(float deltatime) override;

private:
    float sensitivity_ = 0.3F;
    float move_speed_ = 1.0F;
};

} // namespace engine
