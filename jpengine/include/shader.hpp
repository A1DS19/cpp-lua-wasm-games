#pragma once
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
    [[nodiscard]] GLuint get_id() const { return shader_program_; }

private:
    GLuint shader_program_;
};
} // namespace jpengine
