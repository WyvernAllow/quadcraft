#version 450

layout (location = 0) in vec3 v_color;
layout (location = 1) in vec2 v_uv;

layout (location = 0) out vec4 v_frag;

layout (set = 2, binding = 0) uniform sampler2D u_sampler;

void main() {
    v_frag = texture(u_sampler, v_uv);
}