#include "utils/timer.hpp"

#include <chrono>

using namespace jpengine::utils;
using namespace std::chrono;

void Timer::start() {
    if (!is_running_) {
        start_point_ = steady_clock::now();
        is_running_ = true;
        is_paused_ = false;
    }
}

void Timer::stop() {
    if (is_running_) {
        is_running_ = false;
    }
}

void Timer::pause() {
    if (is_running_ && !is_paused_) {
        is_paused_ = true;
        paused_point_ = steady_clock::now();
    }
}

void Timer::resume() {
    if (is_running_ && is_paused_) {
        is_paused_ = false;
        start_point_ += duration_cast<milliseconds>(steady_clock::now() - paused_point_);
    }
}

void Timer::restart() {
    start_point_ = steady_clock::now();
    is_running_ = true;
    is_paused_ = false;
}

[[nodiscard]] int64_t Timer::elapsed_ms() const {
    if (is_running_) {
        if (is_paused_) {
            return duration_cast<milliseconds>(paused_point_ - start_point_).count();
        }

        return duration_cast<milliseconds>(steady_clock::now() - start_point_).count();
    }

    return 0;
}

[[nodiscard]] int64_t Timer::elapsed_sec() const {
    return elapsed_ms() / 1000;
}
