#include "rendering/shader.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace jpengine;

Shader::Shader(GLuint program) : shader_program_(program) {}

Shader::~Shader() {
    glDeleteProgram(shader_program_);
}

void Shader::enable() {
    glUseProgram(shader_program_);
}

void Shader::disable() {
    glUseProgram(0);
}

void Shader::set_uniform_mat4(std::string_view name, glm::mat4& matrix) {
    glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]);
}

[[nodiscard]] glm::mat4 Shader::get_uniform_mat4(std::string_view name) {
    GLfloat values[16];
    glGetUniformfv(shader_program_, get_uniform_location(name), values);
    return glm::make_mat4(values);
}

GLint Shader::get_uniform_location(std::string_view name) {
    auto uniform_itr = uniform_locations_.find(name);
    if (uniform_itr != uniform_locations_.end()) {
        return uniform_itr->second;
    }

    GLint location = glGetUniformLocation(shader_program_, name.data());
    if (location == -1) {
        std::cerr << "uniform [" << name << "] not found in shader.\n";
        return 0;
    }

    uniform_locations_.emplace(name, location);
    return location;
}
