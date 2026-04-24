#pragma once

#include "engine/engine.hpp"
#include "engine/src/graphics/shader-program.hpp"
#include "engine/src/render/material.hpp"
#include "engine/src/render/mesh.hpp"
#include "engine/src/render/render-queue.hpp"

#include <memory>

class Game : public engine::Application {
public:
    bool init() override;
    void update(float deltatime = 0.0F) override;
    void destroy() override;

private:
    std::shared_ptr<engine::ShaderProgram> pshader_program_;
    engine::Material material_;
    std::unique_ptr<engine::Mesh> mesh_;
};
