#include "engine/src/engine.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace engine {

namespace {
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr const char* WINDOW_TITLE = "jpengine-3d";
} // namespace

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

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "failed to init GLEW\n";
        glfwDestroyWindow(pwindow_);
        pwindow_ = nullptr;
        glfwTerminate();
        return false;
    }

    return papplication_->init();
}

void Engine::run() {
    if (!papplication_ || !pwindow_) {
        return;
    }

    last_time_point_ = std::chrono::high_resolution_clock::now();

    while (!papplication_->needs_to_be_closed() && !glfwWindowShouldClose(pwindow_)) {
        glfwPollEvents();

        auto now = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float>(now - last_time_point_).count();
        last_time_point_ = now;

        papplication_->update(delta_time);

        glfwSwapBuffers(pwindow_);
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
