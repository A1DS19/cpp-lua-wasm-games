#include "engine/src/graphics/shader-program.hpp"

#include "engine/engine.hpp"
#include "engine/src/graphics/texture.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace engine {
void ShaderProgram::bind() {
    glUseProgram(shader_program_id_);
    current_texture_unit_ = 0;
}

GLint ShaderProgram::get_uniform_location(const std::string& name) {
    auto it = uniform_location_cache_.find(name);

    if (it != uniform_location_cache_.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(shader_program_id_, name.c_str());
    uniform_location_cache_[name] = location;

    return location;
}

void ShaderProgram::set_uniform(const std::string& name, float value) {

    auto location = get_uniform_location(name);
    glUniform1f(location, value);
}

void ShaderProgram::set_uniform(const std::string& name, float v0, float v1) {
    auto location = get_uniform_location(name);
    glUniform2f(location, v0, v1);
}

void ShaderProgram::set_uniform(const std::string& name, const glm::mat4& mat) {
    auto location = get_uniform_location(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void ShaderProgram::set_uniform(const std::string& name, glm::vec3 value) {
    auto location = get_uniform_location(name);
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::set_texture(const std::string& name, Texture* texture) {
    auto location = get_uniform_location(name);
    glActiveTexture(GL_TEXTURE0 + current_texture_unit_);
    glBindTexture(GL_TEXTURE_2D, texture->get_id());
    glUniform1i(location, current_texture_unit_);

    ++current_texture_unit_;
}

} // namespace engine
