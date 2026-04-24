#pragma once
#include "engine/engine.hpp"
#include "engine/src/scene/game-object.hpp"

#include <memory>

class TestObject : public engine::GameObject {
public:
    TestObject();
    void update(float deltatime) override;
};
