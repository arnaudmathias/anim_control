#version 410 core
layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec3 vert_normal;
layout (location = 2) in ivec4 vert_bone_ids;
layout (location = 3) in vec4 vert_weights;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 B[128];

out vec3 frag_pos;
out vec3 frag_normal;

void main() {
  mat4 bone_transform = B[vert_bone_ids[0]] * vert_weights[0];
  bone_transform += B[vert_bone_ids[1]] * vert_weights[1];
  bone_transform += B[vert_bone_ids[2]] * vert_weights[2];
  bone_transform += B[vert_bone_ids[3]] * vert_weights[3];

  gl_Position = MVP * (bone_transform * vec4(vert_pos, 1.0));
  frag_pos = vec3(M * vec4(vert_pos, 1.0));
  frag_normal = vec3(MVP * (bone_transform * vec4(vert_normal, 0.0)));
}
