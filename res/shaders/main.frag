#version 450
layout (location = 0) out vec4 v_frag;

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;

layout (set = 2, binding = 0) uniform sampler2D u_sampler;

void main() {
    vec2 uv = fract(vec2(dot(v_normal.xzy, v_position.zxx), v_position.y + v_normal.y * v_position.z));

    v_frag = texture(u_sampler, uv);
}