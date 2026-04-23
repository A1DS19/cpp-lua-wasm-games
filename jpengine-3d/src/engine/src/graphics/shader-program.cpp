#include "engine/src/graphics/shader-program.hpp"

#include "engine/engine.hpp"

namespace engine {
void ShaderProgram::bind() {}

GLint ShaderProgram::get_uniform_location(const std::string& name) {
    auto it = uniform_location_cache_.find(name);

    if (it != uniform_location_cache_.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(shader_program_id_, name.c_str());
    uniform_location_cache_[name] = location;

    return location;
}

} // namespace engine
