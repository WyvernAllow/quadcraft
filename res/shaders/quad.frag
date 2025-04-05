#version 430
layout(location = 0) out vec4 v_color;

in vec2 v_texcoord;

uniform sampler2D u_texture;

void main() {
	v_color = texture(u_texture, v_texcoord);
}