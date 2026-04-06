#pragma once

#include <sol/state.hpp>

namespace jpengine {

struct ScriptFuncBinder {
    static void create_lua_bind(sol::state& lua);
};
} // namespace jpengine
