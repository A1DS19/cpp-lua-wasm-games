#pragma once

#include "engine/engine.hpp"

class SuzanneObject : public engine::GameObject {
public:
    SuzanneObject();
    void update(float deltatime) override;
};
