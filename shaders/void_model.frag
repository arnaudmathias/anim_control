#version 410 core

in vec3 frag_pos;

out vec4 frag_color;

void main() {
	vec3 uv = floor(frag_pos + 0.5f);
	if (mod(uv.x + uv.y + uv.z, 2.0f) == 0.0f) {
		frag_color = vec4(0.6, 0.6, 0.6, 1.0);
	} else {
		frag_color = vec4(0.4, 0.4, 0.4, 1.0);
	}
}
