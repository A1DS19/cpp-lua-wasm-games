#pragma once
#include <glm/ext/matrix_float4x4.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#ifdef __APPLE__
    #include <OpenGL/gl3.h>
#else
    #include <GLES3/gl3.h>
#endif

namespace jpengine {
class Shader {
public:
    Shader(GLuint program);
    ~Shader();
    void enable();
    void disable();

    void set_uniform_mat4(std::string_view name, glm::mat4& matrix);
    [[nodiscard]] glm::mat4 get_uniform_mat4(std::string_view name);
    [[nodiscard]] GLuint get_id() const { return shader_program_; }

private:
    GLuint shader_program_;
    std::unordered_map<std::string_view, GLint> uniform_locations_;

    GLint get_uniform_location(std::string_view name);
};
} // namespace jpengine
