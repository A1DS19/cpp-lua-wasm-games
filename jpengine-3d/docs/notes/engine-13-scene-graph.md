# 13 — Scene Graph and Components

> Previous: [12 — Loading meshes from glTF](./engine-12-mesh-loading.md) · Next: [14 — Roadmap](./engine-14-roadmap.md)

The original demo had `Game` directly holding `mesh_`, `material_`, `pshader_program_` as members. That works for one rectangle. As soon as you have a camera, two cubes, and a Suzanne, the game class becomes a junk drawer.

This note covers the abstraction that replaces it:

- `Scene` — owns a tree of `GameObject`s and ticks them.
- `GameObject` — has a transform and a list of `Component`s.
- `Component` — base class for behaviors attached to a GameObject.
- The `COMPONENT(...)` macro and runtime type IDs.

---

## Why a scene graph

A few problems show up the moment you need more than one entity:

| Problem | Symptom | Fix |
|---|---|---|
| Per-entity state in `Game` | `Game.hpp` grows to 200 fields | move state into self-contained classes |
| Identical setup repeated | "another cube" needs another mesh + material handle | factor a class that does its own setup |
| Hierarchy (camera follows player) | manual transform multiplication every frame | parent/child pointers + cached world transforms |
| Iterating "everything that's alive" | `for_each(member)` in the game class | a flat container the engine walks |
| Reusing logic | "this one rotates, this one bounces, this one shoots" | components — composable behaviors |

Each fix points at the same architecture: a tree of objects with attached behaviors. That's a scene graph + an entity-component pattern.

---

## `Scene`

```cpp
class Scene {
public:
    void update(float dt);
    void clear();

    GameObject* create_object(std::string name, GameObject* parent = nullptr);

    // Templated factory for derived types:
    template <std::derived_from<GameObject> T>
    T* create_object(std::string name, GameObject* parent = nullptr);

    bool set_parent(GameObject* object, GameObject* parent);

    void set_main_camera(GameObject* camera) { main_camera_ = camera; }
    [[nodiscard]] GameObject* get_main_camera() const { return main_camera_; }

private:
    std::vector<std::unique_ptr<GameObject>> objects_;   // root-level objects
    GameObject* main_camera_ = nullptr;
};
```

The Scene owns root-level objects via `unique_ptr`. Children of those objects are owned by their parent (`GameObject::children_`). One ownership chain from the Scene down through every nested GameObject — never two parents holding the same child.

### `create_object<T>` is in the header

The template definition has to live in `scene.hpp`:

```cpp
template <std::derived_from<GameObject> T>
T* create_object(std::string name, GameObject* parent = nullptr) {
    auto* object = new T();
    object->set_name(std::move(name));
    set_parent(object, parent);
    return object;
}
```

If you put this in `scene.cpp`, the linker errors with `Undefined symbol: Scene::create_object<TestObject>`. Templates are instantiated where they're called from, so the compiler needs to *see* the body when compiling `game.cpp`. The non-template overload is fine to keep in `scene.cpp`.

### `std::derived_from<T>` (C++20 concept)

The `<concepts>` header gives you `std::derived_from`. It enforces at compile time that `T` actually inherits from `GameObject`:

```cpp
scene.create_object<TestObject>("cube");      // ok
scene.create_object<int>("oops");             // compile error: int is not derived_from<GameObject>
```

Better diagnostics than the old SFINAE-with-`std::enable_if` trick, and the constraint is part of the function signature so tooling shows it on hover.

---

## `GameObject`

```cpp
class GameObject {
public:
    virtual ~GameObject() = default;
    virtual void update(float dt);

    // Transform — stored as pos + quat + scale, composed lazily.
    glm::vec3& get_position();
    glm::quat& get_rotation();
    glm::vec3& get_scale();
    void set_position(glm::vec3);
    void set_rotation(glm::quat);
    [[nodiscard]] glm::mat4 get_local_transform() const;
    [[nodiscard]] glm::mat4 get_world_transform() const;

    // Components.
    void add_component(Component* component);
    template <std::derived_from<Component> T>
    T* get_component();

protected:
    GameObject() = default;

private:
    std::string name_;
    GameObject* parent_ = nullptr;
    std::vector<std::unique_ptr<GameObject>> children_;
    std::vector<std::unique_ptr<Component>>  components_;
    glm::vec3 position_ = glm::vec3(0.0F);
    glm::quat rotation_ = glm::quat(1, 0, 0, 0);   // identity
    glm::vec3 scale_    = glm::vec3(1.0F);

    friend class Scene;
};
```

Three observations:

- **Rotation is a quaternion**, not euler angles. Quaternions don't suffer gimbal lock and compose cleanly. To rotate, multiply by a delta: `rotation_ = delta * rotation_;` (see `glm::angleAxis`).
- **`children_` and `components_` are `unique_ptr`** so destructors recurse correctly. Drop the GameObject and everything beneath it goes with it.
- **`add_component(Component*)`** takes a raw pointer that becomes `unique_ptr` inside — same legacy factory-handoff pattern we discussed in [the unique_ptr note](https://github.com/A1DS19/cpp-lua-wasm-games). Caller does `add_component(new MeshComponent(...))`. A safer API is `add_component<MeshComponent>(args...)` — see roadmap.

### Transforms

```cpp
glm::mat4 GameObject::get_local_transform() const {
    return glm::translate(glm::mat4(1.0F), position_) *
           glm::mat4_cast(rotation_) *
           glm::scale(glm::mat4(1.0F), scale_);
}

glm::mat4 GameObject::get_world_transform() const {
    if (parent_) return parent_->get_world_transform() * get_local_transform();
    return get_local_transform();
}
```

`world = parent_world * local`. A child's transform is composed up the chain on demand. For a player whose hand follows their arm: hand has a small local offset; arm rotates; the hand's world position automatically follows.

This is recomputed every frame for now — fine for a few hundred objects. At thousands you'd cache the world matrix and invalidate on parent change.

---

## `Component`

```cpp
class Component {
public:
    virtual ~Component() = default;
    virtual void update(float dt) = 0;
    [[nodiscard]] virtual size_t get_type_id() const = 0;
    [[nodiscard]] GameObject* get_owner() { return owner_; }

    template <typename T>
    static size_t static_type_id() {
        static const size_t id = next_id_++;
        return id;
    }

protected:
    GameObject* owner_ = nullptr;

private:
    static inline size_t next_id_ = 0;
    friend class GameObject;
};
```

Each component subclass:

- Gets a unique runtime integer ID via `static_type_id<T>()`.
- Implements `update(dt)` — called every frame by its owner.
- Knows its owner via `owner_` (set when added to a GameObject).

### The `COMPONENT(...)` macro

```cpp
#define COMPONENT(ComponentClass)                                       \
public:                                                                 \
    static size_t type_id() {                                           \
        return Component::static_type_id<ComponentClass>();             \
    }                                                                   \
    [[nodiscard]] size_t get_type_id() const override {                 \
        return type_id();                                               \
    }
```

In every component subclass:

```cpp
class MeshComponent : public Component {
    COMPONENT(MeshComponent)
public:
    void update(float dt) override;
    // …
};
```

Two methods, both returning the same number, but reachable two different ways:

- **`MeshComponent::type_id()`** — static, ask "what ID is this *type*?" — used as a lookup key.
- **`mesh_component->get_type_id()`** — virtual, ask "what type is *this instance*?" — used to identify components through a `Component*` base pointer.

The static-local trick in `static_type_id<T>` gives each `T` a unique `id` initialized once on first call. No RTTI, no compile-time assignment — just a counter that increments per-type at first lookup.

### Why not RTTI?

`typeid(*c) == typeid(MeshComponent)` works too, but:

- RTTI lookups compare strings (or are slower than an integer compare).
- RTTI emits a `type_info` blob per polymorphic class — binary bloat.
- Cross-DLL `typeid` is unreliable on some platforms.

For per-frame component lookups (potentially thousands per frame), an integer compare wins. Most game engines roll their own type-ID system for this reason — Unreal `UObject`, EnTT, Unity ECS all do something similar.

### `GameObject::get_component<T>`

```cpp
template <std::derived_from<Component> T>
T* get_component() {
    size_t type_id = Component::static_type_id<T>();
    for (auto& c : components_) {
        if (c->get_type_id() == type_id) {
            return static_cast<T*>(c.get());
        }
    }
    return nullptr;
}
```

Linear scan, but components are usually 1–10 per object. If you ever profile and find this hot, switch to `unordered_map<size_t, unique_ptr<Component>>`.

---

## Built-in components

| Component | Job |
|---|---|
| `MeshComponent` | Holds a `Mesh*` and `Material*`; submits a render command each frame. |
| `CameraComponent` | Computes view & projection matrices; the `Engine`'s render path queries the active camera. |
| `PlayerControllerComponent` | Reads input and translates the owner GameObject. |

Each one is `update(dt) override` plus whatever fields it needs. Adding a component is a self-contained 2-file change.

---

## A typical setup

In `Game::init`:

```cpp
scene_ = new engine::Scene();

// Camera object.
auto* camera = scene_->create_object("camera");
camera->add_component(new engine::CameraComponent());
camera->add_component(new engine::PlayerControllerComponent());
camera->set_position(glm::vec3(0.0F, 0.0F, 4.0F));
scene_->set_main_camera(camera);

// Procedural cube on the left.
auto* cube = scene_->create_object<TestObject>("cube");
cube->set_position(glm::vec3(-1.5F, 0.0F, 0.0F));

// Suzanne (loaded from glTF) on the right.
auto* suzanne = scene_->create_object<SuzanneObject>("suzanne");
suzanne->set_position(glm::vec3(1.5F, 0.0F, 0.0F));

engine::Engine::get_instance().set_scene(scene_);
```

Each "kind" of game object is its own class. `TestObject::TestObject()` builds the cube mesh and attaches a `MeshComponent`; `SuzanneObject::SuzanneObject()` calls `Mesh::load` and attaches a `MeshComponent`. The Game itself stops being a junk drawer — it just declares what's in the scene.

---

## Frame flow

```
Engine::run loop frame:
  glfwPollEvents               (callbacks update InputManager)
  Application::update(dt)
     └─▶ scene_->update(dt)
            └─▶ for each root object:
                   if (alive) object->update(dt)
                        └─▶ for each component: component->update(dt)
                        └─▶ for each child: child->update(dt) (recursive)
  RenderQueue::draw            (drains submitted commands)
  glfwSwapBuffers
```

Components submit render commands during their `update`. The render queue drains them after the scene has finished updating. That separation keeps "what to draw" and "how to draw" in different code paths — see [doc 07](./engine-07-render-queue.md).

---

## What could go wrong

1. **Component type-ID collision.** All `*_index_` constants set to 0 in `VertexElement` (or all components returning the same ID). Lookup returns the wrong component. Make sure each subclass has a unique ID via the macro.

2. **Forgot to register the camera.** `Engine::run` queries `scene_->get_main_camera()` for view/projection. If `set_main_camera` was never called, the engine renders without a transform — everything sits at the origin.

3. **Stack-allocated GameObject passed to `set_parent`.** The Scene takes ownership via `unique_ptr`. If you pass `&local_object`, the destructor will try to `delete` a stack address. Crash. Always create through `Scene::create_object<T>`, which `new`s internally.

4. **Cyclic parent chain.** `set_parent` checks for cycles before changing parents and returns `false` if you'd create one. A silent return value — log it if you need to know.

5. **`add_component(component)` after the owner is destroyed.** Holding a raw `Component*` past its owner's lifetime gives you a dangling pointer. The component is owned by the GameObject, not by you.

---

## See also

- [02 — Main loop](./engine-02-main-loop.md) — where `update(dt)` originates.
- [07 — Render queue](./engine-07-render-queue.md) — what `MeshComponent` submits to.
- [12 — Mesh loading](./engine-12-mesh-loading.md) — supplies the `Mesh` that `MeshComponent` holds.
