#pragma once

#include "glm/ext/matrix_float4x4.hpp"

#include <glew/include/GL/glew.h>
#include <string>
#include <unordered_map>

namespace engine {

class Texture;
class ShaderProgram {

public:
    ShaderProgram() = delete;
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&&) = delete;

    explicit ShaderProgram(GLuint shader_program_id) : shader_program_id_{shader_program_id} {}
    ~ShaderProgram() { glDeleteProgram(shader_program_id_); }

    void bind();
    GLint get_uniform_location(const std::string& name);
    void set_uniform(const std::string& name, float value);
    void set_uniform(const std::string& name, float v0, float v1);
    void set_uniform(const std::string& name, const glm::mat4& mat);
    void set_texture(const std::string& name, Texture* texture);

private:
    std::unordered_map<std::string, GLint> uniform_location_cache_;
    GLuint shader_program_id_ = 0;
    int current_texture_unit_ = 0;
};

} // namespace engine
