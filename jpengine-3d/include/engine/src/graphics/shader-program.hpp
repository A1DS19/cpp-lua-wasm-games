#pragma once

#include <glew/include/GL/glew.h>
#include <string>
#include <unordered_map>

namespace engine {

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

private:
    std::unordered_map<std::string, GLint> uniform_location_cache_;
    GLuint shader_program_id_ = 0;
};

} // namespace engine
