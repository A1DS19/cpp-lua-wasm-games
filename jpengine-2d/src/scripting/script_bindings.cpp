#include "scripting/script_bindings.hpp"

#include "utils/timer.hpp"

#include <SDL_timer.h>
#include <iostream>
#include <sol/error.hpp>
#include <sol/raii.hpp>

using namespace jpengine;

void ScriptFuncBinder::create_lua_bind(sol::state& lua) {
    lua.set_function("j2d_run_script", [&](const std::string& path) {
        try {
            lua.safe_script_file(path);
        } catch (const sol::error& error) {
            std::cerr << "Error loading Lua Script: " << error.what() << std::endl;
            return false;
        }

        return true;
    });

    lua.set_function("j2d_load_script_table", [&](const sol::table& script_list) {
        if (!script_list.valid()) {
            std::cerr << "Failed to load script list: Table is invalid.\n";
            return;
        }

        for (const auto& [index, script] : script_list) {
            try {
                auto result = lua.safe_script_file(script.as<std::string>());
                if (!result.valid()) {
                    sol::error error = result;
                    throw error;
                }
            } catch (const sol::error& error) {
                std::cerr << "Failed to load script: " << script.as<std::string>()
                          << ", Error: " << error.what() << std::endl;
                return;
            }
        }
    });

    lua.set_function("J2D_GetTicks", [] { return SDL_GetTicks(); });

    lua.new_usertype<utils::Timer>(
        "Timer", sol::call_constructor, sol::constructors<utils::Timer()>(), "start",
        &utils::Timer::start, "stop", &utils::Timer::stop, "resume", &utils::Timer::resume, "pause",
        &utils::Timer::pause, "is_running", &utils::Timer::is_running, "is_paused",
        &utils::Timer::is_paused, "elapsed_ms", &utils::Timer::elapsed_ms, "elapsed_sec",
        &utils::Timer::elapsed_sec);
}
