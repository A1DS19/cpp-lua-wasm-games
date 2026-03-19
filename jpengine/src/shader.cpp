#include "shader.hpp"

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
