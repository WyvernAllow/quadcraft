#version 450

layout(location = 0) in vec3 a_color;

layout(location = 0) out vec4 v_frag;

void main() {
    v_frag = vec4(a_color, 1.0);
}