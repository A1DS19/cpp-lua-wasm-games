#pragma once

#include <box2d/b2_body.h>
#include <box2d/box2d.h>
#include <memory>

namespace jpengine {

using PhysicsWorld = std::shared_ptr<b2World>;
struct BodyDestroyer {
    void operator()(b2Body* pbody) const;
};

inline std::shared_ptr<b2Body> make_shared_body(b2Body* pbody) {
    return std::shared_ptr<b2Body>(pbody, BodyDestroyer{});
}

} // namespace jpengine
