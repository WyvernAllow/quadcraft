#version 430
layout(location = 0) out vec4 v_color;

in vec2 v_texcoord;

void main() {
	v_color = vec4(fract(v_texcoord), 0.0, 1.0);
}