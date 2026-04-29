#pragma once
#include "engine/src/application.hpp"
#include "engine/src/graphics/graphics-api.hpp"
#include "engine/src/graphics/texture.hpp"
#include "engine/src/input/input-manager.hpp"
#include "engine/src/render/render-queue.hpp"
#include "engine/src/scene/scene.hpp"

#include <chrono>
#include <memory>

struct GLFWwindow;

namespace engine {
class Engine {
public:
    static Engine& get_instance() {
        static Engine instance;
        return instance;
    }

    bool init();
    void run();
    void destroy();

    void set_application(Application* papplication) noexcept { papplication_.reset(papplication); }
    [[nodiscard]] Application* get_application() noexcept { return papplication_.get(); }
    [[nodiscard]] GLFWwindow* get_window() noexcept { return pwindow_; }
    [[nodiscard]] InputManager& get_input_manager() { return input_manager_; }
    [[nodiscard]] GraphicsApi& get_graphics_api() { return graphics_api_; }
    [[nodiscard]] RenderQueue& get_render_queue() { return render_queue_; }
    void set_scene(Scene* scene) { current_scene_.reset(scene); }
    [[nodiscard]] Scene* get_current_scene() { return current_scene_.get(); }
    [[nodiscard]] TextureManager& get_texture_manager() { return texture_manager_; }

private:
    Engine() = default;
    Engine(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;

    std::unique_ptr<Application> papplication_;
    std::chrono::steady_clock::time_point last_time_point_;
    GLFWwindow* pwindow_ = nullptr;
    InputManager input_manager_;
    GraphicsApi graphics_api_;
    RenderQueue render_queue_;
    std::unique_ptr<Scene> current_scene_;
    TextureManager texture_manager_;
};

} // namespace engine
