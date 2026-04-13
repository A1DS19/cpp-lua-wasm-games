#pragma once

#include <box2d/b2_collision.h>
#include <box2d/b2_contact.h>
#include <box2d/box2d.h>

namespace jpengine {

struct UserData;

class ContactListener : public b2ContactListener {

public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
    void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

private:
    void set_user_contacts(UserData* a, UserData* b);

private:
    UserData* puser_data_a_{nullptr};
    UserData* puser_data_b_{nullptr};
};

} // namespace jpengine
