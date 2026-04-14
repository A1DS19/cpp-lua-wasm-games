#pragma once

#include <cstdint>

namespace jpengine {

enum class RigidBodyType {
    // zero mass, zero velocity, may be manually moved
    STATIC = 0,
    // zero mass, velocity set by user, moved by solver
    KINEMATIC,
    // positive mass, velocity determined by forces, moved by solver
    DYNAMIC
};

enum class FilterCategory : std::uint16_t {
    NO_CATEGORY = 0,
    PLAYER = 1 << 0,
    ENEMY = 1 << 1,
    ITEM = 1 << 2,
    WALLS = 1 << 3,
    GROUND = 1 << 4,
    TRIGGER = 1 << 5,
    PROYECTILE = 1 << 6,
    CATEGORY_7 = 1 << 7,
    CATEGORY_8 = 1 << 8,
    CATEGORY_9 = 1 << 9,
    CATEGORY_10 = 1 << 10,
    CATEGORY_11 = 1 << 11,
    CATEGORY_12 = 1 << 12,
    CATEGORY_13 = 1 << 13,
    CATEGORY_14 = 1 << 14,
    CATEGORY_15 = 1 << 15,
};

} // namespace jpengine
