#include "physics/contact-listener.hpp"

#include "physics/user-data.hpp"

#include <any>
#include <cstddef>

namespace jpengine {

void ContactListener::BeginContact(b2Contact* contact) {
    auto* fixture_a = contact->GetFixtureA();
    auto* fixture_b = contact->GetFixtureB();

    if (!fixture_a || !fixture_b || !fixture_a->GetUserData().pointer ||
        !fixture_b->GetUserData().pointer) {
        set_user_contacts(nullptr, nullptr);
        return;
    }

    UserData* a_data = reinterpret_cast<UserData*>(fixture_a->GetUserData().pointer);
    UserData* b_data = reinterpret_cast<UserData*>(fixture_b->GetUserData().pointer);

    constexpr auto expected_type = entt::type_hash<ObjectData>::value();

    if (!a_data || !b_data || a_data->type_id_ != expected_type ||
        b_data->type_id_ != expected_type) {
        set_user_contacts(nullptr, nullptr);
        return;
    }

    try {
        auto* a_any = std::any_cast<ObjectData>(&a_data->user_data_);
        auto* b_any = std::any_cast<ObjectData>(&b_data->user_data_);

        if (!a_any || !b_any) {
            set_user_contacts(nullptr, nullptr);
        }

        a_any->add_contact(b_any);
        b_any->add_contact(a_any);

        set_user_contacts(a_data, b_data);

    } catch (const std::bad_any_cast& ex) {
        set_user_contacts(nullptr, nullptr);
    }
}

void ContactListener::EndContact(b2Contact* contact) {
    auto* fixture_a = contact->GetFixtureA();
    auto* fixture_b = contact->GetFixtureB();

    if (!fixture_a || !fixture_b || !fixture_a->GetUserData().pointer ||
        !fixture_b->GetUserData().pointer) {
        set_user_contacts(nullptr, nullptr);
        return;
    }

    UserData* a_data = reinterpret_cast<UserData*>(fixture_a->GetUserData().pointer);
    UserData* b_data = reinterpret_cast<UserData*>(fixture_b->GetUserData().pointer);

    constexpr auto expected_type = entt::type_hash<ObjectData>::value();

    if (!a_data || !b_data || a_data->type_id_ != expected_type ||
        b_data->type_id_ != expected_type) {
        set_user_contacts(nullptr, nullptr);
        return;
    }

    try {
        auto* a_any = std::any_cast<ObjectData>(&a_data->user_data_);
        auto* b_any = std::any_cast<ObjectData>(&b_data->user_data_);

        if (!a_any && b_any) {
            b_any->clear_contacts();
            set_user_contacts(nullptr, nullptr);
            return;
        }

        if (a_any && !b_any) {
            a_any->clear_contacts();
            set_user_contacts(nullptr, nullptr);
            return;
        }

        a_any->remove_contact(b_any);
        b_any->remove_contact(a_any);

    } catch (const std::bad_any_cast& ex) {}

    set_user_contacts(nullptr, nullptr);
}

void ContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {}

void ContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {}

void ContactListener::set_user_contacts(UserData* a, UserData* b) {
    puser_data_a_ = a;
    puser_data_b_ = b;
}

} // namespace jpengine
