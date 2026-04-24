#include "engine/src/graphics/graphics-api.hpp"

#include "engine/src/graphics/shader-program.hpp"
#include "engine/src/render/material.hpp"
#include "engine/src/render/mesh.hpp"

#include <GL/glew.h>
#include <iostream>

namespace engine {

namespace {

GLuint compile_shader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::string log(static_cast<std::size_t>(length), '\0');
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        std::cerr << "shader compile failed: " << log << '\n';
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

} // namespace

std::shared_ptr<ShaderProgram>
GraphicsApi::create_shader_program(const std::string& vertex_source,
                                   const std::string& fragment_source) {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    if (vertex_shader == 0) {
        return nullptr;
    }

    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    if (fragment_shader == 0) {
        glDeleteShader(vertex_shader);
        return nullptr;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::string log(static_cast<std::size_t>(length), '\0');
        glGetProgramInfoLog(program, length, nullptr, log.data());
        std::cerr << "shader link failed: " << log << '\n';
        glDeleteProgram(program);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return nullptr;
    }

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return std::make_shared<ShaderProgram>(program);
}

void GraphicsApi::bind_shader_program(ShaderProgram* pshader_program) {
    if (pshader_program) {
        pshader_program->bind();
    }
}

void GraphicsApi::bind_material(Material* pmaterial) {
    if (pmaterial) {
        pmaterial->bind();
    }
}

uint32_t GraphicsApi::create_vertex_buffer(const std::vector<float>& vertices) {

    GLuint buffer_id = 0; // vbo
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);
    return buffer_id;
}

uint32_t GraphicsApi::create_index_buffer(const std::vector<uint32_t>& indices) {
    GLuint buffer_id = 0; // ebo
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)), indices.data(),
                 GL_STATIC_DRAW);
    return buffer_id;
}

void GraphicsApi::bind_mesh(Mesh* pmesh) {
    if (pmesh) {
        pmesh->bind();
    }
}

void GraphicsApi::draw_mesh(Mesh* pmesh) {
    if (pmesh) {
        pmesh->draw();
    }
}

} // namespace engine
