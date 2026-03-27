#include "ecs/entity.hpp"

#include "ecs/registry.hpp"
#include "utils/meta-utils.hpp"

#include <cstdint>
#include <sol/forward.hpp>
#include <sol/raii.hpp>
#include <sol/state.hpp>
#include <sol/types.hpp>

using namespace jpengine;

void Entity::create_lua_bind(sol::state& lua, Registry& registry) {
    using namespace entt::literals;

    lua.new_usertype<Entity>(
        "Entity", sol::call_constructor,
        sol::factories([](Registry& reg) {
        return Entity{reg}; },
                       [&registry]() {
        return Entity{registry}; },
                       [&registry](std::uint32_t id) {
        return Entity{registry, static_cast<entt::entity>(id)};
                       }),
        "add_component",
        [](Entity& entity, const sol::table& comp, sol::this_state state) -> sol::object {
        if (!comp.valid()) {
            return sol::object{};
        }

        const auto component = utils::invoke_meta_function(utils::get_id_type(comp),
                                                           "add_component"_hs, entity, comp, state);

        return component ? component.cast<sol::object>() : sol::object{};
        },

        "has_component",
        [](Entity& entity, const sol::table& comp) {
        if (!comp.valid()) {
            return false;
        }

        const auto has_comp =
            utils::invoke_meta_function(utils::get_id_type(comp), "has_component"_hs, entity);

        return has_comp ? has_comp.cast<bool>() : false;
    },
            "get_component", [](Entity& entity, const sol::table& comp, sol::this_state state) -> sol::object {
        if (!comp.valid()) {
            return sol::object{};
        }
        const auto component = utils::invoke_meta_function(utils::get_id_type(comp),
                                                           "get_component"_hs, entity, state);

        return component ? component.cast<sol::object>() : sol::object{};
    },
            "remove_component",
        [](Entity& entity, const sol::table& comp) -> bool {
        if (!comp.valid()) {
            return false;
        }

        const auto removed_comp =
            utils::invoke_meta_function(utils::get_id_type(comp), "has_component"_hs, entity);

        return removed_comp ? removed_comp.cast<bool>() : false;
    },
    "destroy", &Entity::destroy,
    "id", [](Entity& entity){
        return static_cast<std::uint32_t>(entity.get_entity());}
    );
}
