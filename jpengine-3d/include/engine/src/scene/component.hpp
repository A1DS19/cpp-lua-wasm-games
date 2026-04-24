#pragma once

namespace engine {

class GameObject;
class Component {

public:
    virtual ~Component() = default;
    virtual void update(float deltatime) = 0;
    [[nodiscard]] GameObject* get_owner() { return owner_; }

protected:
    GameObject* owner_ = nullptr;

    friend class GameObject;
};

} // namespace engine
