#pragma once
#include <chrono>
#include <cstdint>

namespace jpengine::utils {

class Timer {
public:
    Timer() = default;
    ~Timer() = default;

    void start();
    void stop();
    void pause();
    void resume();
    void restart();

    [[nodiscard]] int64_t elapsed_ms() const;
    [[nodiscard]] int64_t elapsed_sec() const;

    [[nodiscard]] bool is_running() const { return is_running_; }
    [[nodiscard]] bool is_paused() const { return is_running_; }

private:
    std::chrono::time_point<std::chrono::steady_clock> start_point_;
    std::chrono::time_point<std::chrono::steady_clock> paused_point_;
    bool is_running_{false};
    bool is_paused_{false};
};

} // namespace jpengine::utils
