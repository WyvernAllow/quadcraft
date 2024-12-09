#include <spdlog/spdlog.h>

#include <cstdlib>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static void glfw_error_callback(int error_code, const char* description) {
    spdlog::error("GLFW: {}", description);
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        spdlog::critical("glfwInit failed");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window =
        glfwCreateWindow(800, 450, "Quadcraft", nullptr, nullptr);
    if (!window) {
        spdlog::critical("glfwCreateWindow failed");
        return EXIT_FAILURE;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}