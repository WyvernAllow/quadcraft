#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static void glfw_error_callback(int error, const char* description) {
    spdlog::error("GLFW: {}", description);
}

static void gl_debug_callback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar* message, const void* userParam) {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        spdlog::error("OpenGL: {}", message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        spdlog::warn("OpenGL: {}", message);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        spdlog::error("OpenGL: {}", message);
        break;
    default:
        spdlog::debug("OpenGL: {}", message);
        break;
    }
}

static void glfw_framebuffer_size_callback(GLFWwindow* window, int width,
                                           int height) {
    glViewport(0, 0, width, height);
}

static std::string read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error(
            fmt::format("Failed to open file: {}", filename));
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::string buffer(file_size, '\0');
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}

static GLuint compile_shader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint did_compile;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &did_compile);
    if (!did_compile) {
        char info_log[512];
        glGetShaderInfoLog(shader, sizeof(info_log), nullptr, info_log);

        throw std::runtime_error(
            fmt::format("Shader compilation failed: {}", info_log));
    }

    return shader;
}

static GLuint link_program(const std::vector<GLuint>& shaders) {
    GLuint program = glCreateProgram();
    for (const auto& shader : shaders) {
        glAttachShader(program, shader);
    }

    glLinkProgram(program);

    GLint did_link;
    glGetProgramiv(program, GL_LINK_STATUS, &did_link);
    if (!did_link) {
        char info_log[512];
        glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);

        throw std::runtime_error(
            fmt::format("Program linking failed: {}", info_log));
    }

    for (const auto& shader : shaders) {
        glDetachShader(program, shader);
    }

    return program;
}

struct vertex {
    glm::vec2 position;
    glm::vec2 texcoord;
};

static constexpr size_t WINDOW_W = 800;
static constexpr size_t WINDOW_H = 450;

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        spdlog::error("glfwInit failed");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    GLFWwindow* window =
        glfwCreateWindow(WINDOW_W, WINDOW_H, "Quadcraft", nullptr, nullptr);
    if (!window) {
        spdlog::error("glfwCreateWindow failed");
        return EXIT_FAILURE;
    }

    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) {
        spdlog::error("gladLoadGL failed");
        return EXIT_FAILURE;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);

    const std::string vert_source = read_file("res/shaders/quad.vert");
    const std::string frag_source = read_file("res/shaders/quad.frag");

    GLuint vert = compile_shader(vert_source.c_str(), GL_VERTEX_SHADER);
    GLuint frag = compile_shader(frag_source.c_str(), GL_FRAGMENT_SHADER);

    GLuint quad = link_program({vert, frag});
    glDeleteShader(vert);
    glDeleteShader(frag);

    std::vector<vertex> vertices = {
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f}, {1.0f, 1.0f}},
        {{-1.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f}, {1.0f, 0.0f}},
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 1, 3, 2,
    };

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, texcoord));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    const std::string pathtracer_src = read_file("res/shaders/pathtracer.comp");
    GLuint pathtracer_shader =
        compile_shader(pathtracer_src.c_str(), GL_COMPUTE_SHADER);
    GLuint pathtracer = link_program({pathtracer_shader});
    glDeleteShader(pathtracer_shader);

    GLuint output_image;
    glGenTextures(1, &output_image);
    glBindTexture(GL_TEXTURE_2D, output_image);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, WINDOW_W, WINDOW_H);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Dispatch compute shader
        glUseProgram(pathtracer);
        glBindImageTexture(0, output_image, 0, GL_FALSE, 0, GL_WRITE_ONLY,
                           GL_RGBA32F);
        glDispatchCompute(std::ceil(WINDOW_W / 8), std::ceil(WINDOW_H / 4), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                        GL_TEXTURE_FETCH_BARRIER_BIT);
        glUseProgram(0);

        // Render the output image to the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(quad);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, output_image);
        glUniform1i(glGetUniformLocation(quad, "u_texture"), 0);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()),
                       GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(quad);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}