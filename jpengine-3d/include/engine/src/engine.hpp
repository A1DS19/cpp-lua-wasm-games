#pragma once
#include "engine/src/application.hpp"

#include <chrono>
#include <memory>

struct GLFWwindow;

namespace engine {
class Engine {
public:
    bool init();
    void run();
    void destroy();

    void set_application(Application* papplication) noexcept { papplication_.reset(papplication); }
    [[nodiscard]] Application* get_application() noexcept { return papplication_.get(); }

    [[nodiscard]] GLFWwindow* get_window() noexcept { return pwindow_; }

private:
    std::unique_ptr<Application> papplication_;
    std::chrono::steady_clock::time_point last_time_point_;
    GLFWwindow* pwindow_ = nullptr;
};

} // namespace engine
