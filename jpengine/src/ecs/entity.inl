#include "ecs/entity.hpp"
#include "ecs/registry.hpp"

#include <functional>
#include <sol/make_reference.hpp>
#include <utility>

using namespace jpengine;

template <typename TComponent, typename... Args>
TComponent& Entity::add_component(Args&&... args) {
    auto& registry = registry_.get_registry();
    return registry.emplace<TComponent>(entity_, std::forward<Args>(args)...);
}

template <typename TComponent, typename... Args>
TComponent& Entity::replace_component(Args&&... args) {
    auto& registry = registry_.get_registry();

    if (registry.all_of<TComponent>(entity_)) {
        return registry.replace<TComponent>(entity_, std::forward<Args>(args)...);
    }

    return registry.emplace<TComponent>(entity_, std::forward<Args>(args)...);
}

template <typename TComponent>
TComponent& Entity::get_component() {
    auto& registry = registry_.get_registry();
    return registry.get<TComponent>(entity_);
}

template <typename TComponent>
TComponent* Entity::try_get_component() {
    auto& registry = registry_.get_registry();
    return registry.try_get<TComponent>(entity_);
}

template <typename TComponent>
bool Entity::has_component() {
    auto& registry = registry_.get_registry();
    return registry.all_of<TComponent>(entity_);
}

template <typename TComponent>
auto Entity::remove_component() {
    auto& registry = registry_.get_registry();
    return registry.remove<TComponent>(entity_);
}

template <typename TComponent>
void Entity::register_meta_component() {
    using namespace entt::literals;

    entt::meta_factory<TComponent>()
        .type(entt::type_hash<TComponent>::value())
        .template func<&jpengine::add_component<TComponent>>("add_component"_hs)
        .template func<&jpengine::get_component<TComponent>>("get_component"_hs)
        .template func<&jpengine::has_component<TComponent>>("has_component"_hs)
        .template func<&jpengine::remove_component<TComponent>>("remove_component"_hs);
}

namespace jpengine {

template <typename TComponent>
auto add_component(Entity& entity, const sol::table& comp, sol::this_state state) {
    auto& component = entity.add_component<TComponent>(
        comp.valid() ? std::move(comp.as<TComponent&&>()) : TComponent{});

    return sol::make_reference(state, std::ref(component));
}

template <typename TComponent>
auto get_component(Entity& entity, sol::this_state state) {
    auto* pcomp = entity.try_get_component<TComponent>();
    return pcomp ? sol::make_reference(state, std::ref(*pcomp)) : sol::lua_nil_t{};
}

template <typename TComponent>
bool has_component(Entity& entity) {
    return entity.has_component<TComponent>();
}

template <typename TComponent>
auto remove_component(Entity& entity) {
    entity.remove_component<TComponent>();
}

} // namespace jpengine
