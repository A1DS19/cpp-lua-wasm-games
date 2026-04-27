#pragma once

#include <cstddef>

namespace engine {

class GameObject;

class Component {

public:
    virtual ~Component() = default;
    virtual void update(float deltatime) = 0;
    [[nodiscard]] virtual size_t get_type_id() const = 0;
    [[nodiscard]] GameObject* get_owner() { return owner_; }

    template <typename T>
    static size_t static_type_id() {
        static const size_t type_id = next_id_++;
        return type_id;
    }

protected:
    GameObject* owner_ = nullptr;

private:
    static inline size_t next_id_ = 0;

    friend class GameObject;
};

// Inject type-ID glue into a derived component class. Names must match the
// snake_case API on `Component` (static_type_id / get_type_id).
#define COMPONENT(ComponentClass)                                                                  \
public:                                                                                            \
    static size_t type_id() {                                                                      \
        return Component::static_type_id<ComponentClass>();                                        \
    }                                                                                              \
    [[nodiscard]] size_t get_type_id() const override {                                            \
        return type_id();                                                                          \
    }

} // namespace engine
