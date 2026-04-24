# 03 — Input

> Previous: [02 — Main loop](./engine-02-main-loop.md) · Next: [04 — Graphics API](./engine-04-graphics-api.md)

## Why this exists

Input hardware is asynchronous. The OS tells you about key presses *when they happen*, not on your game's frame boundary. GLFW exposes this via **callbacks** — you register a function pointer, and GLFW calls it during `glfwPollEvents()` for each event.

That's fine for "the A key was pressed" notifications, but awkward for "is A currently held down?" Gameplay code runs on the frame loop and wants to *poll* state like "is W held", "is space being held right now". You don't want gameplay scattered across callback functions.

The fix is a two-layer pattern:

1. **Callback layer** — GLFW calls us when events happen. We store the result in a buffer.
2. **Polling layer** — gameplay reads from that buffer each frame.

That buffer, plus the methods to read/write it, is the `InputManager`.

---

## The `InputManager` class

`include/engine/src/input/input-manager.hpp`:

```cpp
namespace engine {

class InputManager {
public:
    void set_key_pressed(int key_code, bool pressed);
    bool is_key_pressed(int key_code);

private:
    InputManager() = default;
    // deleted copy/move — only the Engine creates this

    friend class Engine;
    std::array<bool, 256> keys_ = {false};
};

}
```

That's it. An array of 256 bools keyed by GLFW key code, two methods to write and read.

**Why 256?** ASCII fits in a byte, and GLFW's key codes for letters/digits/symbols fall within that range. Non-printable keys (F1–F12, arrows, modifiers) have codes above 256, so you can't reach them with this array. That's a known limitation — see the end.

**Why private constructor + friend?** The `Engine` owns an `InputManager` by value. Nobody else should be able to construct one (there's global state associated with it via GLFW's callback). The `friend` declaration lets `Engine` construct the one instance it needs; the private default ctor prevents anyone else from doing so.

---

## Wiring GLFW's callback to the manager

Two pieces connect GLFW events to the `InputManager`:

### 1. A free function that GLFW can call

`src/engine/src/engine.cpp`:

```cpp
void key_callback(GLFWwindow*, int key, int, int action, int) {
    auto& input = engine::Engine::get_instance().get_input_manager();
    if (action == GLFW_PRESS) {
        input.set_key_pressed(key, true);
    } else if (action == GLFW_RELEASE) {
        input.set_key_pressed(key, false);
    }
}
```

Points to notice:

- **GLFW's signature is fixed**: `void (*)(GLFWwindow*, int, int, int, int)`. The callback must match exactly. Unused parameters are left *unnamed* (their names would just collect `-Wunused-parameter` warnings; and since this build treats warnings as errors, that would fail the build).
- **It's a free function, not a method.** GLFW's C API can't call a C++ member function directly — it needs a plain function pointer. We reach the manager via the engine singleton.
- **We only translate PRESS and RELEASE.** GLFW also emits `GLFW_REPEAT` when the OS auto-repeats a held key. We ignore it because we're modeling "is currently held," not "how many press events."

### 2. Registration during engine init

`Engine::init` calls:

```cpp
glfwSetKeyCallback(pwindow_, key_callback);
```

From this point on, every `glfwPollEvents()` (called at the top of each frame) will drain any pending key events through `key_callback`, which updates `keys_` inside the `InputManager`.

---

## Using it from game code

```cpp
void Game::update(float) {
    auto& input = engine::Engine::get_instance().get_input_manager();

    if (input.is_key_pressed(GLFW_KEY_W)) offset_y += 0.01F;
    if (input.is_key_pressed(GLFW_KEY_A)) offset_x -= 0.01F;
    if (input.is_key_pressed(GLFW_KEY_S)) offset_y -= 0.01F;
    if (input.is_key_pressed(GLFW_KEY_D)) offset_x += 0.01F;
}
```

No callbacks in sight — just polling the stored state. If W is held, every frame `is_key_pressed(W)` returns `true` and the offset keeps accumulating. That's what we want for continuous movement.

**`GLFW_KEY_W` is a constant** defined in `<GLFW/glfw3.h>`. It happens to be `87` (ASCII `'W'`), which fits in the 256-slot array.

---

## Implementation detail: bounds checking

`src/engine/src/input/input-manager.cpp`:

```cpp
void InputManager::set_key_pressed(int key_code, bool pressed) {
    if (key_code < 0 || key_code >= static_cast<int>(keys_.size())) {
        return;
    }
    keys_[static_cast<std::size_t>(key_code)] = pressed;
}
```

Two things worth understanding:

- **The bounds check uses `||`, not `&&`.** Early drafts of this code had `&&`, which is always false (no value is simultaneously < 0 and ≥ size). That bug meant out-of-range GLFW key codes (like `GLFW_KEY_F1` = 290) would silently write past the end of the array — undefined behavior. Easy mistake to make.
- **The cast is mandatory.** `std::array::operator[]` takes `std::size_t`. The build flags include `-Werror -Wsign-conversion`, so assigning an `int` index without an explicit cast is a compile error.

---

## What's naive about this

- **Key-press-once is hard.** `is_key_pressed` returns true every frame the key is held. If you want "fire one bullet when the player taps space," you need a second layer that compares current state with previous state. Or you keep a separate list of *events* for that frame.
- **No mouse, gamepad, or text input.** Only keyboard. Mouse would be another pair of methods (`get_mouse_position`, `is_mouse_button_pressed`) with another callback. Text input (for typing in a UI) wants GLFW's character callback, not the key callback.
- **256-key ceiling.** F1–F12, arrow keys, and modifiers all have codes > 256. Fix is either a bigger array (GLFW key codes max around 348) or a `std::unordered_map<int, bool>`. A map is more general but slower; a 512-slot array is probably the right compromise.
- **No key remapping layer.** Gameplay code is hardcoded to specific physical keys. A "real" input system usually maps physical inputs to game actions ("move_left" instead of "A key"), so the player can rebind or so the same code works with a gamepad.
- **Callback reaches through the singleton.** If you ever wanted two windows with separate input, this would need rethinking — GLFW's callback gets a `GLFWwindow*` that we currently ignore.

---

## See also

- [02 — Main loop](./engine-02-main-loop.md) — where `glfwPollEvents()` fits.
- [08 — Walkthrough](./engine-08-walkthrough.md) — the WASD offset demo in full.
- [GLFW input guide](https://www.glfw.org/docs/latest/input_guide.html) — upstream docs.
