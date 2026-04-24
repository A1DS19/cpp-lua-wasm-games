#pragma once

#include <memory>
#include <string>
#include <unordered_map>
namespace engine {

class ShaderProgram;
class Material {

public:
    void set_shader_program(const std::shared_ptr<ShaderProgram>& pshader_program) {
        pshader_program_ = pshader_program;
    }
    void set_param(const std::string& name, float param_value) {
        float_params_[name] = param_value;
    }
    void bind();

private:
    std::shared_ptr<ShaderProgram> pshader_program_;
    std::unordered_map<std::string, float> float_params_;
};

} // namespace engine
