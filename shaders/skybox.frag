#version 410 core

in vec3 frag_pos;

out vec4 frag_color;

uniform samplerCube skybox;

void main() {
    frag_color = texture(skybox, frag_pos);
}
