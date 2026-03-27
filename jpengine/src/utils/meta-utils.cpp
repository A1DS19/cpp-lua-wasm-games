#include "utils/meta-utils.hpp"

#include "ecs/component.hpp"

#include <cassert>
#include <iostream>
#include <sol/forward.hpp>

[[nodiscard]] entt::id_type jpengine::utils::get_id_type(const sol::table& comp) {
    if (!comp.valid()) {
        std::cerr << "failed to get type id component has not been exposed to lua\n";
        assert(comp.valid() && "failed to get type id component has not been exposed to lua\n");
        return entt::id_type(-1);
    }

    const auto func = comp["type_id"].get<sol::function>();
    assert(func.valid() && "[type_id()] function has not been exposed to lua");

    return func.valid() ? func().get<entt::id_type>() : entt::id_type(-1);
}
