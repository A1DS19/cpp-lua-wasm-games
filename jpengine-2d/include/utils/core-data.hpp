#pragma once

#include <cstdint>
namespace jpengine {

class CoreData {
public:
    static CoreData& get_instance();
    // make no copy
    CoreData(const CoreData&) = delete;
    CoreData& operator=(const CoreData&) = delete;

    [[nodiscard]] float meters_to_pixels() const noexcept;
    [[nodiscard]] float pixels_to_meters() const noexcept;

    void enable_physics() noexcept { physic_enabled_ = true; }
    void disable_physics() noexcept { physic_enabled_ = false; }
    void pause_physics() noexcept { physic_paused_ = true; }
    void unpause_physics() noexcept { physic_paused_ = false; }
    [[nodiscard]] bool is_physics_enabled() const noexcept { return physic_enabled_; }
    [[nodiscard]] bool is_physics_paused() const noexcept { return physic_paused_; }

    void set_window_w(float window_w) noexcept;
    void set_window_h(float window_h) noexcept;
    void set_scaled_w(float scaled_w) noexcept;
    void set_scaled_h(float scaled_h) noexcept;

    void set_velocity_iterations(std::int32_t velocity_iterations) noexcept {
        velocity_iterations_ = velocity_iterations;
    }
    void set_position_iterations(std::int32_t position_iterations) noexcept {
        position_iterations_ = position_iterations;
    }
    void set_gravity(float gravity) noexcept { gravity_ = gravity; }

    [[nodiscard]] float get_scaled_w() const noexcept { return scaled_w_; }
    [[nodiscard]] float get_scaled_h() const noexcept { return scaled_h_; }
    [[nodiscard]] float get_gravity() const noexcept { return gravity_; }
    [[nodiscard]] std::int32_t get_velocity_iterations() const noexcept {
        return velocity_iterations_;
    }
    [[nodiscard]] std::int32_t get_position_iterations() const noexcept {
        return position_iterations_;
    }

private:
    CoreData() noexcept;
    ~CoreData() = default;

private:
    // w of screen in meters
    float scaled_w_;
    // h of screen in meters
    float scaled_h_;
    // default gravity force
    float gravity_;
    // w of screen in pixels
    float window_w_;
    // h of screen in pixels
    float window_h_;
    // velocity tuning parameter for accuracy in box2d
    std::int32_t velocity_iterations_;
    // position tuning parameter for accuracy in position_iterations_
    std::int32_t position_iterations_;
    // flag to toggle physics on/off
    bool physic_enabled_;
    // flag to pause physics
    bool physic_paused_;
};

} // namespace jpengine
