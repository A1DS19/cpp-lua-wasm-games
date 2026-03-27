#pragma once

#include "ecs/component.hpp"

#include <cassert>
#include <iostream>
#include <sol/table.hpp>

namespace jpengine::utils {
[[nodiscard]] entt::id_type get_id_type(const sol::table& comp);
template <typename... Args>
inline auto invoke_meta_function(entt::meta_type meta, entt::id_type func_id, Args&&... args) {
    if (!meta) {
        std::cerr << "no entt::meta_type has been provided or is invalid\n";
        assert(false && "no entt::meta_type has been provided or is invalid\n");

        return entt::meta_any{};
    }

    if (auto meta_func = meta.func(func_id); meta_func) {
        return meta_func.invoke({}, std::forward<Args>(args)...);
    }

    std::cerr << "no meta.func has been provided or is invalid\n";
    assert(false && "no meta.func has been provided or is invalid\n");

    return entt::meta_any{};
}

template <typename... Args>
inline auto invoke_meta_function(entt::id_type id, entt::id_type func_id, Args... args) {
    return invoke_meta_function(entt::resolve(id), func_id, std::forward<Args>(args)...);
}
} // namespace jpengine::utils
