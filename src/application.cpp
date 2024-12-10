#include "application.hpp"

#include <stdexcept>

application::application() {
    if (!glfwInit()) {
        throw std::runtime_error("glfwInit failed");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(800, 450, "Quadcraft", nullptr, nullptr);
    if (!m_window) {
        throw std::runtime_error("glfwCreateWindow failed");
    }
}

application::~application() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void application::run() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}