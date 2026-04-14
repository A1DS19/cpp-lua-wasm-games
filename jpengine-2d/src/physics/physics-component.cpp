#include "physics/physics-component.hpp"

#include "physics/box2d-wrappers.hpp"
#include "physics/user-data.hpp"
#include "utils/core-data.hpp"

#include <any>
#include <box2d/b2_body.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

namespace jpengine {

PhysicsComponent::PhysicsComponent() : PhysicsComponent(PhysicsAttributes{}) {}

PhysicsComponent::PhysicsComponent(const PhysicsAttributes& physics_attributes)
    : prigid_body_{nullptr}, puser_data_{nullptr}, initial_attribs_{physics_attributes} {}

PhysicsComponent::~PhysicsComponent() = default;

void PhysicsComponent::init(PhysicsWorld pphysics_world, int window_w, int window_h) {
    if (!pphysics_world) {
        std::cerr << "failed to get initialized the physics component - physics world is invalid\n";
        return;
    }

    bool is_circle{initial_attribs_.bcircle_};

    // create body def
    b2BodyDef body_def{};
    body_def.type = static_cast<b2BodyType>(initial_attribs_.etype_);
    float pixels_to_meters = CORE_DATA().pixels_to_meters();

    // set initial position of body
    body_def.position.Set(
        (initial_attribs_.position_.x + initial_attribs_.offset_.x - (window_w * 0.5F) +
         ((is_circle ? initial_attribs_.radius_ : initial_attribs_.box_size_.x) *
          initial_attribs_.scale_.x) *
             0.5F) *
            pixels_to_meters,

        (initial_attribs_.position_.y + initial_attribs_.offset_.y - (window_h * 0.5F) +
         ((is_circle ? initial_attribs_.radius_ : initial_attribs_.box_size_.y) *
          initial_attribs_.scale_.y) *
             0.5F) *
            pixels_to_meters);

    body_def.gravityScale = initial_attribs_.gravity_scale_;
    body_def.fixedRotation = initial_attribs_.bfixed_rotation_;

    prigid_body_ = make_shared_body(pphysics_world->CreateBody(&body_def));
    if (!prigid_body_) {
        std::cerr << "failed to create rigid body\n";
        return;
    }

    b2PolygonShape poly_shape;
    b2CircleShape circle_shape;

    if (is_circle) {
        circle_shape.m_radius =
            pixels_to_meters * initial_attribs_.radius_ * initial_attribs_.scale_.x;
    } else if (initial_attribs_.bbox_shape_) {
        poly_shape.SetAsBox(
            pixels_to_meters * initial_attribs_.box_size_.x * initial_attribs_.scale_.x * 0.5F,
            pixels_to_meters * initial_attribs_.box_size_.y * initial_attribs_.scale_.y * 0.5F);
    } else {
        // add polygons
    }

    // create user data
    puser_data_ = std::make_shared<UserData>();
    puser_data_->user_data_ = initial_attribs_.object_data_;
    puser_data_->type_id_ = entt::type_hash<ObjectData>::value();

    // create fixture def
    b2FixtureDef fixture_def{};
    if (is_circle) {
        fixture_def.shape = &circle_shape;
    } else {
        fixture_def.shape = &poly_shape;
    }

    fixture_def.density = initial_attribs_.density_;
    fixture_def.friction = initial_attribs_.friction_;
    fixture_def.restitution = initial_attribs_.restitution_;
    fixture_def.isSensor = initial_attribs_.bsensor_;
    fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(puser_data_.get());

    auto pfixture = prigid_body_->CreateFixture(&fixture_def);
    assert(pfixture && "fixture was created successfully");
    if (pfixture) {
        std::cerr << "failed to create rigid body fixture\n";
        return;
    }
}

bool PhysicsComponent::is_sensor() const noexcept {
    if (!prigid_body_) {
        return false;
    }

    return prigid_body_->GetFixtureList()->IsSensor();
}

ObjectData PhysicsComponent::cast_ray(const b2Vec2& point_1, const b2Vec2& point_2) const noexcept {
    return {};
}

std::vector<ObjectData> PhysicsComponent::box_trace(const b2Vec2& lower_bounds,
                                                    const b2Vec2& upper_bounds) const noexcept {
    return std::vector<ObjectData>({});
}

[[nodiscard]] ObjectData PhysicsComponent::get_current_object_data() const noexcept {
    if (!prigid_body_) {
        return {};
    }

    UserData* pdata =
        reinterpret_cast<UserData*>(prigid_body_->GetFixtureList()->GetUserData().pointer);

    try {
        auto object_data = std::any_cast<ObjectData>(pdata->user_data_);
        return object_data;
    } catch (const std::bad_any_cast& ex) {
        std::cerr << "failed to cast object data" << ex.what() << "\n";
    }

    return {};
}

void PhysicsComponent::set_filter_category(std::uint16_t mask) noexcept {}

void PhysicsComponent::set_filter_category() noexcept {}

void PhysicsComponent::set_filter_mask(std::uint16_t mask) noexcept {}

void PhysicsComponent::set_filter_mask() noexcept {}

void PhysicsComponent::set_group_index(std::int16_t index) noexcept {}

void PhysicsComponent::set_group_index() noexcept {}

void PhysicsComponent::create_lua_bind(sol::state& lua, PhysicsWorld& physics_world) {
    lua.new_usertype<ObjectData>(
        "ObjectData", "type_id", &entt::type_hash<ObjectData>::value, sol::call_constructor,
        sol::factories(
            [](const std::string& tag, const std::string& group, bool collider, bool trigger,
               bool is_friendly, std::uint32_t entity_id) {
                return ObjectData{tag, group, collider, is_friendly, trigger, entity_id};
            },
            [](const sol::table& tbl) {
                return ObjectData{tbl["tag"].get_or(std::string{}),
                                  tbl["group"].get_or(std::string{}),
                                  tbl["collider"].get_or(false),
                                  tbl["is_friendly"].get_or(false),
                                  tbl["trigger"].get_or(false),
                                  tbl["entity_id"].get_or(static_cast<std::uint32_t>(entt::null))};
            }),
        "tag", &ObjectData::tag_, "group", &ObjectData::group_, "collider", &ObjectData::collider_,
        "trigger", &ObjectData::trigger_, "is_friendly", &ObjectData::is_friendly_, "entity_id",
        &ObjectData::entity_id_, "contact_entities",
        sol::readonly_property([](ObjectData& obj) { return obj.get_contact_entities(); }));

    lua.new_enum<RigidBodyType>("BodyType", {
                                                {"Static", RigidBodyType::STATIC},
                                                {"Kinematic", RigidBodyType::KINEMATIC},
                                                {"Dynamic", RigidBodyType::DYNAMIC},
                                            });

    lua.new_usertype<PhysicsAttributes>(
        "PhysicsAttributes", sol::call_constructor,
        sol::factories(
            [] { return PhysicsAttributes{}; },
            [](const sol::table& tbl) {
                return PhysicsAttributes{
                    .etype_ = tbl["etype"].get_or(RigidBodyType::STATIC),
                    .density_ = tbl["density"].get_or(1.F),
                    .friction_ = tbl["friction"].get_or(0.2F),
                    .restitution_ = tbl["restitution"].get_or(0.2F),
                    .radius_ = tbl["radius"].get_or(0.F),
                    .gravity_scale_ = tbl["gravity_scale"].get_or(1.F),
                    .position_ = glm::vec2{tbl["position"]["x"].get_or(0.F),
                                           tbl["position"]["y"].get_or(0.F)},
                    .scale_ =
                        glm::vec2{tbl["scale"]["x"].get_or(1.F), tbl["scale"]["y"].get_or(1.F)},
                    .box_size_ = glm::vec2{tbl["box_size"]["x"].get_or(0.F),
                                           tbl["box_size"]["y"].get_or(0.F)},
                    .offset_ =
                        glm::vec2{tbl["offset"]["x"].get_or(0.F), tbl["offset"]["y"].get_or(0.F)},
                    .bcircle_ = tbl["bcircle"].get_or(false),
                    .bbox_shape_ = tbl["bbox_shape"].get_or(true),
                    .bfixed_rotation_ = tbl["bfixed_rotation"].get_or(true),
                    .bsensor_ = tbl["bsensor"].get_or(false),
                    .bbullet_ = tbl["bbullet"].get_or(false),
                    .buse_filters_ = tbl["buse_filters"].get_or(false),
                    .filter_category_ =
                        tbl["filter_category"].get_or(static_cast<std::uint16_t>(0)),
                    .filter_mask_ = tbl["filter_mask"].get_or(static_cast<std::uint16_t>(0)),
                    .group_index_ = tbl["group_index"].get_or(static_cast<std::int16_t>(0)),
                    .object_data_ = ObjectData{tbl["object_data"]["tag"].get_or(std::string{}),
                                               tbl["object_data"]["group"].get_or(std::string{}),
                                               tbl["object_data"]["collider"].get_or(false),
                                               tbl["object_data"]["is_friendly"].get_or(false),
                                               tbl["object_data"]["trigger"].get_or(false),
                                               tbl["object_data"]["entity_id"].get_or(
                                                   static_cast<std::uint32_t>(0))},
                };
            }),
        "etype", &PhysicsAttributes::etype_, "density", &PhysicsAttributes::density_, "friction",
        &PhysicsAttributes::friction_, "restitution", &PhysicsAttributes::restitution_, "radius",
        &PhysicsAttributes::radius_, "gravity_scale", &PhysicsAttributes::gravity_scale_,
        "position", &PhysicsAttributes::position_, "scale", &PhysicsAttributes::scale_, "box_size",
        &PhysicsAttributes::box_size_, "offset", &PhysicsAttributes::offset_, "bcircle",
        &PhysicsAttributes::bcircle_, "bbox_shape", &PhysicsAttributes::bbox_shape_,
        "bfixed_rotation", &PhysicsAttributes::bfixed_rotation_, "bsensor",
        &PhysicsAttributes::bsensor_, "bbullet", &PhysicsAttributes::bbullet_, "buse_filters",
        &PhysicsAttributes::buse_filters_, "filter_category", &PhysicsAttributes::filter_category_,
        "filter_mask", &PhysicsAttributes::filter_mask_, "group_index",
        &PhysicsAttributes::group_index_, "object_data", &PhysicsAttributes::object_data_);

    if (!physics_world) {
        std::cerr << "Physics world is not valid. Failed to bind physics component.\n";
        return;
    }

    lua.new_usertype<PhysicsComponent>(
        "PhysicsComp", "type_id", &entt::type_hash<PhysicsComponent>::value, sol::call_constructor,
        sol::factories([&physics_world](const PhysicsAttributes& attr) {
            auto& core = CORE_DATA();
            PhysicsComponent pc{attr};
            pc.init(physics_world, static_cast<int>(core.get_scaled_w() * core.meters_to_pixels()),
                    static_cast<int>(core.get_scaled_h() * core.meters_to_pixels()));
            return pc;
        }),
        "linear_impulse",
        [](PhysicsComponent& pc, const glm::vec2& impulse) {
            auto* body = pc.get_body();
            if (!body)
                return;
            body->ApplyLinearImpulse(b2Vec2{impulse.x, impulse.y}, body->GetPosition(), true);
        },
        "angular_impulse",
        [](PhysicsComponent& pc, float impulse) {
            auto* body = pc.get_body();
            if (!body)
                return;
            body->ApplyAngularImpulse(impulse, true);
        },
        "set_linear_velocity",
        [](PhysicsComponent& pc, const glm::vec2& velocity) {
            auto* body = pc.get_body();
            if (!body)
                return;
            body->SetLinearVelocity(b2Vec2{velocity.x, velocity.y});
        },
        "get_linear_velocity",
        [](PhysicsComponent& pc) {
            auto* body = pc.get_body();
            if (!body)
                return glm::vec2{0.F};
            const auto& v = body->GetLinearVelocity();
            return glm::vec2{v.x, v.y};
        },
        "set_angular_velocity",
        [](PhysicsComponent& pc, float angular_velocity) {
            auto* body = pc.get_body();
            if (!body)
                return;
            body->SetAngularVelocity(angular_velocity);
        },
        "get_angular_velocity",
        [](PhysicsComponent& pc) {
            auto* body = pc.get_body();
            if (!body)
                return 0.F;
            return body->GetAngularVelocity();
        },
        "set_gravity_scale",
        [](PhysicsComponent& pc, float gravity_scale) {
            auto* body = pc.get_body();
            if (!body)
                return;
            body->SetGravityScale(gravity_scale);
        },
        "get_gravity_scale",
        [](PhysicsComponent& pc) {
            auto* body = pc.get_body();
            if (!body)
                return 0.F;
            return body->GetGravityScale();
        },
        "set_transform",
        [](PhysicsComponent& pc, const glm::vec2& position) {
            auto* body = pc.get_body();
            if (!body)
                return;
            auto& core = CORE_DATA();
            const float p2m = core.pixels_to_meters();
            const float half_w = core.get_scaled_w() * 0.5F;
            const float half_h = core.get_scaled_h() * 0.5F;
            body->SetTransform(b2Vec2{(position.x * p2m) - half_w, (position.y * p2m) - half_h},
                               0.F);
        },
        "set_body_type",
        [](PhysicsComponent& pc, RigidBodyType etype) {
            auto* body = pc.get_body();
            if (!body)
                return;
            b2BodyType body_type = b2_dynamicBody;
            switch (etype) {
                case RigidBodyType::STATIC:
                    body_type = b2_staticBody;
                    break;
                case RigidBodyType::DYNAMIC:
                    body_type = b2_dynamicBody;
                    break;
                case RigidBodyType::KINEMATIC:
                    body_type = b2_kinematicBody;
                    break;
                default:
                    assert(false && "body type is invalid");
            }
            body->SetType(body_type);
        },
        "set_bullet",
        [](PhysicsComponent& pc, bool bullet) {
            auto* body = pc.get_body();
            if (!body)
                return;
            body->SetBullet(bullet);
        },
        "is_bullet",
        [](PhysicsComponent& pc) {
            auto* body = pc.get_body();
            if (!body)
                return false;
            return body->IsBullet();
        },
        "cast_ray",
        [](PhysicsComponent& pc, const glm::vec2& p1, const glm::vec2& p2, sol::this_state s) {
            auto obj = pc.cast_ray(b2Vec2{p1.x, p1.y}, b2Vec2{p2.x, p2.y});
            return obj.entity_id_ == static_cast<std::uint32_t>(entt::null)
                       ? sol::object{sol::lua_nil_t{}}
                       : sol::make_object(s, obj);
        },
        "box_trace",
        [](PhysicsComponent& pc, const glm::vec2& lower, const glm::vec2& upper,
           sol::this_state s) {
            auto vec = pc.box_trace(b2Vec2{lower.x, lower.y}, b2Vec2{upper.x, upper.y});
            return vec.empty() ? sol::object{sol::lua_nil_t{}} : sol::make_object(s, vec);
        },
        "object_data", [](PhysicsComponent& pc) { return pc.get_current_object_data(); });
}

} // namespace jpengine
