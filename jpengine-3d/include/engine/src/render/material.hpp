#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
namespace engine {

class ShaderProgram;
class Texture;
class Material {

public:
    void set_shader_program(const std::shared_ptr<ShaderProgram>& pshader_program) {
        pshader_program_ = pshader_program;
    }
    void set_param(const std::string& name, float param_value) {
        float_params_[name] = param_value;
    }
    void set_param(const std::string& name, float v0, float v1) {
        float_2_params_[name] = {v0, v1};
    }
    void set_param(const std::string& name, std::shared_ptr<Texture> ptexture) {
        textures_[name] = std::move(ptexture);
    }
    void bind();
    ShaderProgram* get_shader_program() { return pshader_program_.get(); }

private:
    std::shared_ptr<ShaderProgram> pshader_program_;
    std::unordered_map<std::string, float> float_params_;
    std::unordered_map<std::string, std::pair<float, float>> float_2_params_;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;
};

} // namespace engine
