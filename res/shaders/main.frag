#version 450
layout (location = 0) out vec4 v_frag;

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_atlas_offset;

layout (set = 2, binding = 0) uniform sampler2D u_sampler;

const float tex_size = 16.0;
const vec3 light_dir = normalize(vec3(-0.5, -1.0, -0.5));
const vec3 light_color = vec3(1.0, 0.9, 0.8);

const float ambient_strength = 0.3;

void main() {
    vec2 uv = fract(vec2(dot(v_normal.xzy, v_position.zxx), v_position.y + v_normal.y * v_position.z));

    uv.y = 1.0 - uv.y;

    vec2 tex_coord = (v_atlas_offset * tex_size + uv * tex_size) / textureSize(u_sampler, 0);

    vec3 tex_color = texture(u_sampler, tex_coord).xyz;

    float diffuse_strength = max(dot(v_normal, -light_dir), 0.0);
    vec3 diffuse = diffuse_strength * light_color;

    vec3 ambient = ambient_strength * light_color;

    vec3 result = (ambient + diffuse) * tex_color;

    v_frag = vec4(result, 1.0);
}
