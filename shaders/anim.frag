#version 410 core

in vec3 frag_pos;
in vec3 frag_normal;

out vec4 frag_color;

void main() {
	//frag_color = vec4(1.0, 0.0, 0.0, 1.0);
	frag_color = vec4(normalize(frag_normal), 1.0);
}
