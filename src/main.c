#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camera.h"
#include "lmath.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

static void glfw_error_callback(int error_code, const char *description) {
    (void)error_code;
    fprintf(stderr, "GLFW Error: %s\n", description);
}

static char *read_file_to_str(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror(filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* +1 for null-terminator */
    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        perror(filename);
        return NULL;
    }

    if (fread(buffer, 1, file_size, file) != file_size) {
        perror(filename);
        return NULL;
    }

    /* Null terminate the string */
    buffer[file_size] = '\0';
    return buffer;
}

static GLuint compile_shader(const char *source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint did_compile;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &did_compile);
    if (!did_compile) {
        char info_log[256];
        glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);

        /* Remove any trailing newline characters */
        info_log[strcspn(info_log, "\n")] = 0;
        fprintf(stderr, "glCompileShader failed: %s\n", info_log);

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint compile_program(const char *vert_filename,
                              const char *frag_filename) {
    const char *vert_src = read_file_to_str(vert_filename);
    if (!vert_src) {
        fprintf(stderr, "Failed to load file: %s\n", vert_filename);
        return 0;
    }

    const char *frag_src = read_file_to_str(frag_filename);
    if (!frag_src) {
        fprintf(stderr, "Failed to load file: %s\n", frag_filename);
        return 0;
    }

    GLuint vert = compile_shader(vert_src, GL_VERTEX_SHADER);
    if (!vert) {
        fprintf(stderr, "Failed to compile vertex shader file: %s\n",
                vert_filename);
        return 0;
    }

    GLuint frag = compile_shader(frag_src, GL_FRAGMENT_SHADER);
    if (!frag) {
        fprintf(stderr, "Failed to compile fragment shader file: %s\n",
                frag_filename);
        return 0;
    }

    free((void *)vert_src);
    free((void *)frag_src);

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint did_link;
    glGetProgramiv(program, GL_LINK_STATUS, &did_link);
    if (!did_link) {
        char info_log[256];
        glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);

        /* Remove any trailing newline characters */
        info_log[strcspn(info_log, "\n")] = 0;
        fprintf(stderr, "glLinkProgram failed: %s\n", info_log);

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

typedef struct vertex {
    float position[3];
} vertex;

static camera cam;

static float last_x;
static float last_y;
static bool first_mouse = true;

#define CAM_SPEED 10.0f
#define CAM_SENSITIVITY 0.3f

static void glfw_cursor_pos_callback(GLFWwindow *window, double x, double y) {
    (void)window;

    if (first_mouse) {
        last_x = x;
        last_y = y;
        first_mouse = false;
    }

    float delta_x = x - last_x;
    float delta_y = y - last_y;

    last_x = x;
    last_y = y;

    cam.yaw += to_rad(delta_x * CAM_SENSITIVITY);
    cam.pitch += to_rad(-delta_y * CAM_SENSITIVITY);
}

int main(void) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "glfwInit failed\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800, 450, "Quadcraft", NULL, NULL);
    if (!window) {
        fprintf(stderr, "glfwCreateWindow failed\n");
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "gladLoadGL failed\n");
        return EXIT_FAILURE;
    }

    glfwSwapInterval(0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);

    GLuint program =
        compile_program("res/shaders/main.vert", "res/shaders/main.frag");
    if (!program) {
        fprintf(stderr, "compile_program failed\n");
        return EXIT_FAILURE;
    }

    // clang-format off
    vertex vertices[] = {
       { 0.5f,  0.5f, -1.0f},
       { 0.5f, -0.5f, -1.0f},
       {-0.5f, -0.5f, -1.0f},
       {-0.5f,  0.5f, -1.0f},
    };
    // clang-format on

    uint16_t indices[] = {
        0, 1, 3, 1, 2, 3,
    };

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    /* Position attribute */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (const void *)offsetof(vertex, position));

    glBindVertexArray(0);

    init_camera(&cam, to_rad(85.0f), 800.0f / 450.0f, 0.01f, 1000.0f);

    float current_time = glfwGetTime();
    float last_time = current_time;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        current_time = glfwGetTime();
        float delta_time = current_time - last_time;
        last_time = current_time;

        vec3 wishdir = {0};
        if (glfwGetKey(window, GLFW_KEY_W)) {
            wishdir = vec3_add(wishdir, cam.forward);
        }

        if (glfwGetKey(window, GLFW_KEY_S)) {
            wishdir = vec3_sub(wishdir, cam.forward);
        }

        if (glfwGetKey(window, GLFW_KEY_D)) {
            wishdir = vec3_add(wishdir, cam.right);
        }

        if (glfwGetKey(window, GLFW_KEY_A)) {
            wishdir = vec3_sub(wishdir, cam.right);
        }

        cam.position = vec3_add(
            cam.position,
            vec3_scale(vec3_normalized(wishdir), delta_time * CAM_SPEED));

        update_camera(&cam);

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program, "u_view"), 1, GL_FALSE,
                           cam.view.data);
        glUniformMatrix4fv(glGetUniformLocation(program, "u_proj"), 1, GL_FALSE,
                           cam.proj.data);

        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
