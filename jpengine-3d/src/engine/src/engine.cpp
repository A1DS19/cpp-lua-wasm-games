#include "engine/src/engine.hpp"

#include "engine/src/input/input-manager.hpp"
#include "engine/src/render/render-queue.hpp"
#include "engine/src/scene/components/camera-component.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/detail/qualifier.hpp>
#include <iostream>

namespace engine {

namespace {
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr const char* WINDOW_TITLE = "jpengine-3d";
} // namespace

void key_callback(GLFWwindow*, int key, int, int action, int) {
    auto& input_manager = engine::Engine::get_instance().get_input_manager();
    if (action == GLFW_PRESS) {
        input_manager.set_key_pressed(key, true);
    } else if (action == GLFW_RELEASE) {
        input_manager.set_key_pressed(key, false);
    }
}

void mouse_button_callback(GLFWwindow*, int button, int action, int) {
    auto& input_manager = engine::Engine::get_instance().get_input_manager();
    if (action == GLFW_PRESS) {
        input_manager.set_mouse_pressed(button, true);
    } else if (action == GLFW_RELEASE) {
        input_manager.set_mouse_pressed(button, false);
    }
}

void cursor_position_callback(GLFWwindow* pwindow, double xpos, double ypos) {
    auto& input_manager = engine::Engine::get_instance().get_input_manager();
    input_manager.set_mouse_position_old(input_manager.get_mouse_position_current());
    input_manager.set_mouse_position_current(
        glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos)));
}

bool Engine::init() {
    if (!papplication_) {
        return false;
    }

#ifdef __linux__
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

    if (!glfwInit()) {
        std::cerr << "failed to init GLFW\n";
        return false;
    }

    // Window / context hints must be set BEFORE glfwCreateWindow.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    pwindow_ = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!pwindow_) {
        std::cerr << "failed to create window\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(pwindow_);
    glfwSetKeyCallback(pwindow_, key_callback);
    glfwSetMouseButtonCallback(pwindow_, mouse_button_callback);
    glfwSetCursorPosCallback(pwindow_, cursor_position_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "failed to init GLEW\n";
        glfwDestroyWindow(pwindow_);
        pwindow_ = nullptr;
        glfwTerminate();
        return false;
    }

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return papplication_->init();
}

void Engine::run() {
    if (!papplication_ || !pwindow_) {
        return;
    }

    last_time_point_ = std::chrono::steady_clock::now();

    while (!papplication_->needs_to_be_closed() && !glfwWindowShouldClose(pwindow_)) {
        glfwPollEvents();

        auto now = std::chrono::steady_clock::now();
        float delta_time = std::chrono::duration<float>(now - last_time_point_).count();
        last_time_point_ = now;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        papplication_->update(delta_time);

        CameraData camera_data;
        int width = 0;
        int height = 0;
        glfwGetWindowSize(pwindow_, &width, &height);
        float aspect = static_cast<float>(width) / static_cast<float>(height);

        if (current_scene_) {
            if (auto camera_obj = current_scene_->get_main_camera()) {
                auto camera_component = camera_obj->get_component<CameraComponent>();
                if (camera_component) {
                    camera_data.view_matrix_ = camera_component->get_view_matrix();
                    camera_data.projection_matrix_ =
                        camera_component->get_projection_matrix(aspect);
                }
            }
        }
        render_queue_.draw(graphics_api_, &camera_data);

        glfwSwapBuffers(pwindow_);
        input_manager_.set_mouse_position_old(input_manager_.get_mouse_position_current());
    }
}

void Engine::destroy() {
    if (papplication_) {
        papplication_->destroy();
        papplication_.reset();
    }

    if (pwindow_) {
        glfwDestroyWindow(pwindow_);
        pwindow_ = nullptr;
    }

    glfwTerminate();
}

}; // namespace engine
