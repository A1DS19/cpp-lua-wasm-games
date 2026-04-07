#include "utils/utilities.hpp"

#include "rendering/font.hpp"
#include "utils/asset-manager.hpp"

#include <glm/ext/vector_float2.hpp>
#include <iostream>

using namespace jpengine;
using namespace jpengine::utils;

float JPEngineUtils::measure_text(const std::string& text, Font& font) {
    return static_cast<float>(text.size()) * font.get_font_size();
}

float JPEngineUtils::right_align(const std::string& text, Font& font, const glm::vec2& align_pos) {
    float text_w = measure_text(text, font);
    return align_pos.x - text_w;
}

float JPEngineUtils::center_align(const std::string& text, Font& font, const glm::vec2& align_pos) {
    float text_w = measure_text(text, font);
    return align_pos.x - (text_w * 0.5F);
}

void JPEngineUtils::create_lua_bind(sol::state& lua, AssetManager& asset_manager) {
    lua.set_function("j2d_measure_text", [&](const std::string& text, const std::string& fontname) {
        if (auto pfont = asset_manager.get_font(fontname)) {
            return measure_text(text, *pfont);
        }

        std::cerr << "failed to measure text, font: " << fontname
                  << " does not exist in asset manager\n";
        return 0.F;
    });

    lua.set_function(
        "j2d_right_align_text",
        [&](const std::string& text, const std::string& fontname, const glm::vec2& align_pos) {
            if (auto pfont = asset_manager.get_font(fontname)) {
                return right_align(text, *pfont, align_pos);
            }

            std::cerr << "failed to get right align position, font: " << fontname
                      << " does not exist in asset manager\n";
            return 0.F;
        });

    lua.set_function(
        "j2d_center_align_text",
        [&](const std::string& text, const std::string& fontname, const glm::vec2& align_pos) {
            if (auto pfont = asset_manager.get_font(fontname)) {
                return center_align(text, *pfont, align_pos);
            }

            std::cerr << "failed to get center align position, font: " << fontname
                      << " does not exist in asset manager\n";
            return 0.F;
        });
}
