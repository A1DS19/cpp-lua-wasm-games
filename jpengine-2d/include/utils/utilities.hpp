#pragma once
#include <glm/glm.hpp>
#include <sol/sol.hpp>
#include <sol/state.hpp>
#include <string>

namespace jpengine {
class Font;
class AssetManager;
} // namespace jpengine

namespace jpengine::utils {

class JPEngineUtils {
public:
    // text utils
    static float measure_text(const std::string& text, jpengine::Font& font);
    static float right_align(const std::string& text, jpengine::Font& font, const glm::vec2& align_pos);
    static float center_align(const std::string& text, jpengine::Font& font, const glm::vec2& align_pos);

    static void create_lua_bind(sol::state& lua, jpengine::AssetManager& asset_manager);
};

} // namespace jpengine::utils
