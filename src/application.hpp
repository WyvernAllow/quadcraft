#pragma once
#include <GLFW/glfw3.h>

class application {
   public:
    application();
    ~application();

    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&&) = delete;
    application& operator=(application&&) = delete;

    void run();

   private:
    GLFWwindow* m_window;
};