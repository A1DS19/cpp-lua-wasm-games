#include "ecs/component.hpp"

#include <glm/ext/vector_float2.hpp>

using namespace jpengine;

void ComponentBinder::create_lua_bind(sol::state& lua) {
    lua.new_usertype<Identification>(
        "Identification", "type_id", &entt::type_hash<Identification>::value, sol::call_constructor,
        sol::factories([] { return Identification{}; },
                       [](const std::string& tag, const std::string& group) {
                           return Identification{.tag_ = tag, .group_ = group};
                       }),
        "tag", &Identification::tag_, "group", &Identification::group_, "entity_id",
        &Identification::entity_id_);

    lua.new_usertype<TransformComponent>(
        "Transform", "type_id", &entt::type_hash<TransformComponent>::value, sol::call_constructor,
        sol::factories([] { return TransformComponent{}; },
                       [](const glm::vec2& position, const glm::vec2& scale, float rotation) {
                           return TransformComponent{
                               .position_ = position, .scale_ = scale, .rotation_ = rotation};
                       },
                       [](float x, float y, float rotation) {
                           return TransformComponent{.position_ = glm::vec2{x, y},
                                                     .rotation_ = rotation};
                       }),
        "position", &TransformComponent::position_, "scale", &TransformComponent::scale_,
        "rotation", &TransformComponent::rotation_);

    lua.new_usertype<AnimationComponent>(
        "Animation", "type_id", &entt::type_hash<AnimationComponent>::value, sol::call_constructor,
        sol::factories(
            [](int num_frames, int frame_rate, int frame_offset, bool vertical, bool looped) {
                return AnimationComponent{.new_frames_ = num_frames,
                                          .frame_offset_ = frame_offset,
                                          .frame_rate_ = frame_rate,
                                          .vertical_ = vertical,
                                          .looped_ = looped};
            }),
        "num_frames", &AnimationComponent::new_frames_, "current_frame",
        &AnimationComponent::current_frame_, "frame_offset", &AnimationComponent::frame_offset_,
        "frame_rate", &AnimationComponent::frame_rate_, "start_time",
        &AnimationComponent::start_time_, "vertical", &AnimationComponent::vertical_, "looped",
        &AnimationComponent::looped_, "stop", &AnimationComponent::stop_, "reset",
        &AnimationComponent::reset);

    lua.new_usertype<BoxColider>(
        "BoxCollider", "type_id", &entt::type_hash<BoxColider>::value, sol::call_constructor,
        sol::factories([](int width, int height, glm::vec2 offset) {
            return BoxColider{.width_ = width, .height_ = height, .offset_ = offset};
        }),
        "width", &BoxColider::width_, "height", &BoxColider::height_, "offset",
        &BoxColider::offset_, "active", &BoxColider::collider_);

    lua.new_usertype<CircleCollider>(
        "CircleCollider", "type_id", &entt::type_hash<CircleCollider>::value, sol::call_constructor,
        sol::factories([] { return CircleCollider{}; },
                       [](float radius, const glm::vec2& offset) {
                           return CircleCollider{.radius_ = radius, .offset_ = offset};
                       }),
        "radius", &CircleCollider::radius_, "offset", &CircleCollider::offset_, "trigger",
        &CircleCollider::trigger_, "active", &CircleCollider::collider_);

    lua.new_usertype<SpriteComponent>(
        "Sprite", "type_id", &entt::type_hash<SpriteComponent>::value, sol::call_constructor,
        sol::factories([] { return SpriteComponent{}; },
                       [](const std::string& texture, float width, float height, int layer,
                          int start_x, int start_y, Color color) {
                           return SpriteComponent{.string_ = texture,
                                                  .width_ = width,
                                                  .height_ = height,
                                                  .layer_ = layer,
                                                  .start_x_ = start_x,
                                                  .start_y_ = start_y,
                                                  .color_ = color};
                       }),
        "texture", &SpriteComponent::string_, "width", &SpriteComponent::width_, "height",
        &SpriteComponent::height_, "layer", &SpriteComponent::layer_, "start_x",
        &SpriteComponent::start_x_, "start_y", &SpriteComponent::start_y_, "uvs",
        &SpriteComponent::uvs_, "hidden", &SpriteComponent::hidden_, "color",
        &SpriteComponent::color_, "generate_uvs", &SpriteComponent::generate_uvs, "inspect_uvs",
        &SpriteComponent::inspect_uvs, "inspect_x", &SpriteComponent::inspect_x, "inspect_y",
        &SpriteComponent::inspect_y);

    lua.new_usertype<RigidBodyComponent>(
        "RigidBody", "type_id", &entt::type_hash<RigidBodyComponent>::value, sol::call_constructor,
        sol::factories(
            [] { return RigidBodyComponent{}; },
            [](const glm::vec2& velocity) { return RigidBodyComponent{.velocity_ = velocity}; }),
        "velocity", &RigidBodyComponent::velocity_, "max_velocity",
        &RigidBodyComponent::max_velocity_);
}
