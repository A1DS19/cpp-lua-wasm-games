#include "core/macros.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>

int main() {

#ifdef __linux__
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif // __linux__

    if (!core::macros::convert_to_bool(glfwInit())) {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* pwindow = glfwCreateWindow(1280, 720, "jpengine-3d", nullptr, nullptr);
    glfwMakeContextCurrent(pwindow);

    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    if (!core::macros::convert_to_bool(pwindow)) {
        std::cout << "error creating window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    while (!core::macros::convert_to_bool(glfwWindowShouldClose(pwindow))) {
        glClearColor(1.0F, 0.0F, 0.0F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(pwindow);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
