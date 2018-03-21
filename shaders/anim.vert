#version 410 core
layout (location = 0) in vec3 vert_pos;
layout (location = 1) in ivec4 vert_bone_ids;
layout (location = 2) in vec4 vert_weights;

uniform mat4 MVP;
uniform mat4 M;

out vec3 frag_pos;

void main() {
  gl_Position = MVP * vec4(vert_pos, 1.0);
  frag_pos = vec3(M * vec4(vert_pos, 1.0));
}
