#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include "block_type.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "direction.hpp"

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

static size_t WINDOW_W = 800;
static size_t WINDOW_H = 450;

static void glfw_framebuffer_size_callback(GLFWwindow* window, int width,
                                           int height) {
    WINDOW_W = width;
    WINDOW_H = height;

    glViewport(0, 0, WINDOW_W, WINDOW_H);
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
    glm::vec3 position;
    glm::vec3 normal;
};

camera cam(glm::radians(90.0f), (float)WINDOW_W / WINDOW_H);

bool mouse_locked = false;
float last_x = WINDOW_W / 2.0f;
float last_y = WINDOW_H / 2.0f;

static void glfw_cursor_pos_callback(GLFWwindow* window, double x, double y) {
    if (mouse_locked) {
        float x_offset = static_cast<float>(x) - last_x;
        float y_offset = last_y - static_cast<float>(y);

        cam.yaw += glm::radians(x_offset * 0.3f);
        cam.pitch += glm::radians(y_offset * 0.3f);
    }

    last_x = static_cast<float>(x);
    last_y = static_cast<float>(y);
}

template <typename T>
constexpr size_t vector_nbytes(const std::vector<T>& vector) noexcept {
    return sizeof(T) * vector.size();
}

static const std::vector<uint32_t> generate_quad_indices(size_t quad_count) {
    std::vector<uint32_t> indices(quad_count * 6);
    for (size_t i = 0; i < quad_count; i++) {
        indices[i * 6 + 0] = 1 + i * 4;
        indices[i * 6 + 1] = 3 + i * 4;
        indices[i * 6 + 2] = 2 + i * 4;
        indices[i * 6 + 3] = 3 + i * 4;
        indices[i * 6 + 4] = 1 + i * 4;
        indices[i * 6 + 5] = 0 + i * 4;
    }
    return indices;
}

// clang-format off
static void add_quad(std::vector<vertex>& vertices, const glm::vec3& position, direction dir) {
    glm::vec3 normal = direction_to_vec3(dir);
    switch (dir) {
    case direction::POS_X:
        vertices.push_back({position + glm::vec3(1.0f, 1.0f, 0.0f), normal});  // top right
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f), normal});  // bottom right
        vertices.push_back({position + glm::vec3(1.0f, 0.0f,  1.0f), normal});   // bottom left
        vertices.push_back({position + glm::vec3(1.0f, 1.0f,  1.0f), normal});   // top left
        break;
    case direction::POS_Y:
        vertices.push_back({position + glm::vec3(1.0f, 1.0f, 0.0f), normal});  // top right
        vertices.push_back({position + glm::vec3(1.0f, 1.0f,  1.0f), normal});   // bottom right
        vertices.push_back({position + glm::vec3(0.0f, 1.0f,  1.0f), normal});   // bottom left
        vertices.push_back({position + glm::vec3(0.0f, 1.0f, 0.0f), normal});  // top left
        break;
    case direction::POS_Z:
        vertices.push_back({position + glm::vec3(1.0f, 1.0f, 1.0f), normal});  // top right
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f), normal});  // bottom right
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f), normal});  // bottom left
        vertices.push_back({position + glm::vec3(0.0f, 1.0f, 1.0f), normal});  // top left
        break;
    case direction::NEG_X:
        vertices.push_back({position + glm::vec3(0.0f, 1.0f,  1.0f), normal});   // top right
        vertices.push_back({position + glm::vec3(0.0f, 0.0f,  1.0f), normal});   // bottom right
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f), normal});  // bottom left
        vertices.push_back({position + glm::vec3(0.0f, 1.0f, 0.0f), normal});  // top left
        break;
    case direction::NEG_Y:
        vertices.push_back({position + glm::vec3(1.0f, 0.0f,  1.0f), normal});   // top right
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f), normal});  // bottom right
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f), normal});  // bottom left
        vertices.push_back({position + glm::vec3(0.0f, 0.0f,  1.0f), normal});   // top left
        break;
    case direction::NEG_Z:
        vertices.push_back({position + glm::vec3(0.0f, 1.0f, 0.0f), normal});  // top right
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f), normal});  // bottom right
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f), normal});  // bottom left
        vertices.push_back({position + glm::vec3(1.0f, 1.0f, 0.0f), normal});  // top left
        break;
    }
}

// clang-format on

static std::vector<vertex> mesh_chunk(const chunk& chunk) {
    std::vector<vertex> vertices;

    for (size_t z = 0; z < chunk::SIZE_Z; z++) {
        for (size_t y = 0; y < chunk::SIZE_Y; y++) {
            for (size_t x = 0; x < chunk::SIZE_X; x++) {
                glm::ivec3 position = {x, y, z};

                block_type current = chunk.get_block(position);
                if (current == block_type::AIR) {
                    continue;
                }

                for (size_t i = 0; i < 6; i++) {
                    direction dir = static_cast<direction>(i);
                    glm::ivec3 normal = direction_to_ivec3(dir);

                    block_type neighbor = chunk.get_block(position + normal);

                    if (neighbor == block_type::AIR) {
                        add_quad(vertices, position, dir);
                    }
                }
            }
        }
    }

    return vertices;
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        spdlog::error("glfwInit failed");
        return EXIT_FAILURE;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    WINDOW_W = mode->width;
    WINDOW_H = mode->height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Quadcraft",
                                          glfwGetPrimaryMonitor(), nullptr);
    if (!window) {
        spdlog::error("glfwCreateWindow failed");
        return EXIT_FAILURE;
    }

    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);

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

    init_block_properties();

    std::string vert_source = read_file("res/shaders/quad.vert");
    std::string frag_source = read_file("res/shaders/quad.frag");

    GLuint vert = compile_shader(vert_source.c_str(), GL_VERTEX_SHADER);
    GLuint frag = compile_shader(frag_source.c_str(), GL_FRAGMENT_SHADER);

    GLuint quad = link_program({vert, frag});
    glDeleteShader(vert);
    glDeleteShader(frag);

    chunk chunk;

    // The worst case scenario is a chunk with a checkerboard pattern of blocks.
    // This results in a total of chunk::VOLUME / 2 blocks, with 6 exposed faces
    // each.
    size_t max_quads = (chunk::VOLUME / 2) * 6;

    const std::vector<uint32_t> indices = generate_quad_indices(max_quads);

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, max_quads * 4 * sizeof(vertex), NULL,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vector_nbytes(indices),
                 indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, normal));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    cam.position = glm::vec3(0.0f, 0.0f, 0.0f);

    bool vsync = true;
    glfwSwapInterval(vsync ? 1 : 0);

    float current_time = glfwGetTime();

    bool show_wireframe = false;

    size_t index_count = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float new_time = glfwGetTime();
        float delta_time = new_time - current_time;
        current_time = new_time;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
            if (!mouse_locked) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                mouse_locked = true;
            }
        } else {
            if (mouse_locked) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                mouse_locked = false;
            }
        }

        glm::vec3 wishdir = glm::vec3(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W)) {
            wishdir += cam.forward();
        }
        if (glfwGetKey(window, GLFW_KEY_S)) {
            wishdir -= cam.forward();
        }
        if (glfwGetKey(window, GLFW_KEY_A)) {
            wishdir -= cam.right();
        }
        if (glfwGetKey(window, GLFW_KEY_D)) {
            wishdir += cam.right();
        }

        if (wishdir != glm::vec3(0.0f)) {
            wishdir = glm::normalize(wishdir);
        }

        cam.position += wishdir * delta_time * 5.0f;

        cam.update();

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) && mouse_locked) {
            glm::ivec3 pos = cam.position + cam.forward();

            if (chunk.get_block(pos) != block_type::AIR) {
                chunk.set_block(pos, block_type::AIR);
            }
        }

        if (chunk.dirty()) {
            std::vector<vertex> vertices = mesh_chunk(chunk);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vector_nbytes(vertices),
                            vertices.data());

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            index_count = (vertices.size() / 4) * 6;
            chunk.mark_clean();
        }

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(quad);
        glUniformMatrix4fv(glGetUniformLocation(quad, "u_view"), 1, GL_FALSE,
                           glm::value_ptr(cam.view()));
        glUniformMatrix4fv(glGetUniformLocation(quad, "u_proj"), 1, GL_FALSE,
                           glm::value_ptr(cam.proj()));

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Statistics");
        ImGui::Text("GPU Name: %s", glGetString(GL_RENDERER));
        ImGui::Text("Frame Time: %.1f ms", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Camera position: (%.1f, %.1f, %.1f)", cam.position.x,
                    cam.position.y, cam.position.z);
        ImGui::End();

        ImGui::Begin("Graphics Settings");

        ImGui::SeparatorText("Window");
        if (ImGui::Checkbox("VSync", &vsync)) {
            glfwSwapInterval(vsync ? 1 : 0);
        }

        ImGui::SeparatorText("Camera");
        ImGui::SliderAngle("FOV", &cam.fov, 0.0f, 120.0f);
        ImGui::SliderFloat("Near", &cam.near, 0.01f, 100.0f);
        ImGui::SliderFloat("Far", &cam.far, 0.01f, 1000.0f);

        ImGui::SeparatorText("Rendering");

        if (ImGui::Checkbox("Wireframe", &show_wireframe)) {
            if (show_wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
