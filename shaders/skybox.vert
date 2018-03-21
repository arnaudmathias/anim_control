#version 410 core
layout (location = 0) in vec3 vertex_pos;

out vec3 frag_pos;

uniform mat4 P;
uniform mat4 V;

void main() {
    frag_pos = vertex_pos;
    vec4 pos = P * V * vec4(vertex_pos, 1.0);
    gl_Position = pos.xyww;
}
