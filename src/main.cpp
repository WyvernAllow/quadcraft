#include <spdlog/spdlog.h>

#include <iostream>
#include <stdexcept>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

static constexpr size_t INIT_WINDOW_W = 800;
static constexpr size_t INIT_WINDOW_H = 450;

static void glfw_error_callback(int error_code, const char* description) {
    spdlog::error("GLFW: {}", description);
}

static void run() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        throw std::runtime_error("glfwInit failed");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(INIT_WINDOW_W, INIT_WINDOW_H,
                                          "Quadcraft", nullptr, nullptr);
    if (!window) {
        throw std::runtime_error("glfwCreateWindow failed");
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        throw std::runtime_error("gladLoadGL failed");
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    try {
        run();
    } catch (std::exception& e) {
        spdlog::critical("Fatal exception: {}", e.what());
    }
}