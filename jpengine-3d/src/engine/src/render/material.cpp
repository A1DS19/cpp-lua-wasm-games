#include "engine/src/render/material.hpp"

#include "engine/src/graphics/shader-program.hpp"

namespace engine {
void Material::bind() {
    if (!pshader_program_) {
        return;
    }

    pshader_program_->bind();

    for (auto& param : float_params_) {
        pshader_program_->set_uniform(param.first, param.second);
    }
}
} // namespace engine
