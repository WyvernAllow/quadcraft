#version 450

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

layout (location = 0) out vec3 v_position;
layout (location = 1) out vec3 v_normal;

layout(set = 1, binding = 0) uniform ubo {
    mat4 mvp;
};

void main() {
    v_position = a_position;
    v_normal = a_normal;
    gl_Position = mvp * vec4(a_position, 1.0);
}