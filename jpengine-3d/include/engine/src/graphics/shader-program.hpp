#pragma once

#include <glew/include/GL/glew.h>
#include <string>
#include <unordered_map>

namespace engine {

class ShaderProgram {

public:
    void bind();
    GLint get_uniform_location(const std::string& name);

private:
    std::unordered_map<std::string, GLint> uniform_location_cache_;
    GLuint shader_program_id_ = 0;
};

} // namespace engine
