#include "engine/src/render/material.hpp"

#include "engine/src/engine.hpp"
#include "engine/src/graphics/shader-program.hpp"
#include "engine/src/graphics/texture.hpp"
#include "utils/file-utils.hpp"

#include <json/nlohmann/json.hpp>
#include <memory>
#include <string>

namespace engine {
void Material::bind() {
    if (!pshader_program_) {
        return;
    }

    pshader_program_->bind();

    for (auto& param : float_params_) {
        pshader_program_->set_uniform(param.first, param.second);
    }

    for (auto& param : float_2_params_) {
        pshader_program_->set_uniform(param.first, param.second.first, param.second.second);
    }

    for (auto& texture : textures_) {
        pshader_program_->set_texture(texture.first, texture.second.get());
    }
}

std::shared_ptr<Material> Material::load(const std::string& path) {
    auto contents = utils::read_asset_text(path);
    nlohmann::json json = nlohmann::json::parse(contents);
    std::shared_ptr<Material> result;

    if (json.contains("shader")) {
        auto shader_obj = json["shader"];
        std::string vertex_path = shader_obj.value("vertex", "");
        std::string fragment_path = shader_obj.value("fragment", "");
        auto vertex_source = utils::read_asset_text(vertex_path);
        auto fragment_source = utils::read_asset_text(fragment_path);
        auto& graphics_api = Engine::get_instance().get_graphics_api();
        auto shader_program = graphics_api.create_shader_program(vertex_source, fragment_source);
        if (!shader_program) {
            return nullptr;
        }

        result = std::make_shared<Material>();
        result->set_shader_program(shader_program);
    }

    if (json.contains("params")) {
        auto params_obj = json["params"];

        if (params_obj.contains("float")) {
            for (auto& p : params_obj["float"]) {
                std::string name = p.value("name", "");
                float value = p.value("value", 0.0F);
                result->set_param(name, value);
            }
        }

        if (params_obj.contains("float2")) {
            for (auto& p : params_obj["float2"]) {
                std::string name = p.value("name", "");
                float v0 = p.value("value0", 0.0F);
                float v1 = p.value("value1", 0.0F);
                result->set_param(name, v0, v1);
            }
        }

        if (params_obj.contains("textures")) {
            for (auto& p : params_obj["textures"]) {
                std::string name = p.value("name", "");
                std::string path = p.value("path", "");
                auto texture = Texture::load(path);
                result->set_param(name, texture);
            }
        }
    }

    return result;
}

} // namespace engine
