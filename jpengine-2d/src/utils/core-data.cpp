#include "utils/core-data.hpp"

namespace jpengine {

constexpr float METERS_TO_PIXELS = 12.F;
constexpr float PIXELS_TO_METERS = 1.F / METERS_TO_PIXELS;

CoreData& CoreData::get_instance() {
    static CoreData instance{};
    return instance;
}

CoreData::CoreData() noexcept
    : scaled_w_{0.F}, scaled_h_{0.F}, gravity_{9.8F}, window_w_{640.F}, window_h_{480.F},
      velocity_iterations_{10}, position_iterations_{8}, physic_enabled_{true},
      physic_paused_{false} {
    scaled_w_ = window_w_ / METERS_TO_PIXELS;
    scaled_h_ = window_h_ / METERS_TO_PIXELS;
}

float CoreData::meters_to_pixels() const noexcept {
    return METERS_TO_PIXELS;
}

float CoreData::pixels_to_meters() const noexcept {
    return PIXELS_TO_METERS;
}

void CoreData::set_window_w(float window_w) noexcept {
    window_w_ = window_w;
    scaled_w_ = window_w_ / METERS_TO_PIXELS;
}

void CoreData::set_window_h(float window_h) noexcept {
    window_h_ = window_h;
    scaled_h_ = window_h_ / METERS_TO_PIXELS;
}

void CoreData::set_scaled_w(float scaled_w) noexcept {
    scaled_w_ = scaled_w / METERS_TO_PIXELS;
}

void CoreData::set_scaled_h(float scaled_h) noexcept {
    scaled_h_ = scaled_h / METERS_TO_PIXELS;
}

} // namespace jpengine
