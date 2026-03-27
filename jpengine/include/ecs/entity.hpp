#pragma once
#include "registry.hpp"

#include <sol/table.hpp>
#include <sol/types.hpp>

namespace jpengine {
class Entity {
public:
    Entity(Registry& registry) : registry_{registry}, entity_(registry.create_entity()) {}
    Entity(Registry& registry, const entt::entity& entity) : registry_{registry}, entity_(entity) {}
    Entity(const Entity& other) : registry_{other.registry_}, entity_{other.entity_} {}
    Entity& operator=(const Entity& other) {
        this->entity_ = other.entity_;
        return *this;
    }
    ~Entity() = default;

    std::uint32_t destroy() const noexcept { return registry_.get_registry().destroy(entity_); }
    entt::entity& get_entity() noexcept { return entity_; }

    template <typename TComponent, typename... Args>
    TComponent& add_component(Args&&... args);

    template <typename TComponent, typename... Args>
    TComponent& replace_component(Args&&... args);

    template <typename TComponent>
    TComponent& get_component();

    template <typename TComponent>
    TComponent* try_get_component();

    template <typename TComponent>
    bool has_component();

    template <typename TComponent>
    auto remove_component();

    static void create_lua_bind(sol::state& lua, Registry& registry);

    template <typename TComponent>
    static void register_meta_component();

private:
    Registry& registry_;
    entt::entity entity_;
};

template <typename TComponent>
auto add_component(Entity& entity, const sol::table& comp, sol::this_state state);

template <typename TComponent>
auto get_component(Entity& entity, sol::this_state state);

template <typename TComponent>
bool has_component(Entity& entity);

template <typename TComponent>
auto remove_component(Entity& entity);

} // namespace jpengine

#include "../../src/ecs/entity.inl"
