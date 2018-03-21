#version 410 core
layout (location = 0) in vec4 vert_pos; //xyz = pos, w = face siding
layout (location = 1) in vec2 vert_attrib; //x = texture_id, y = biome_id

uniform mat4 MVP;
uniform mat4 M;

flat out uint texture_id;
flat out vec3 frag_normal;
flat out vec3 frag_tangent;
flat out vec3 frag_bitangent;
flat out uint frag_side;
out vec3 frag_pos;

const vec3 normal_table[6] = vec3[6](
  vec3(0.0, 0.0, 1.0),
  vec3(0.0, 0.0, -1.0),
  vec3(1.0, 0.0, 0.0),
  vec3(-1.0, 0.0, 0.0),
  vec3(0.0, -1.0, 0.0),
  vec3(0.0, 1.0, 0.0)
);

const vec3 tangent_table[6] = vec3[6](
  vec3(-1.0, 0.0, 0.0),
  vec3(1.0, 0.0, 0.0),
  vec3(0.0, 0.0, 1.0),
  vec3(0.0, 0.0, -1.0),
  vec3(1.0, 0.0, 0.0),
  vec3(-1.0, 0.0, 0.0)
);

void main() {
  frag_side = uint(vert_pos.w);
  frag_normal = normal_table[frag_side];
  frag_tangent = tangent_table[frag_side];
  frag_bitangent = cross(frag_normal, frag_tangent);
  texture_id = uint(vert_attrib.x);
  gl_Position = MVP * vec4(vert_pos.xyz, 1.0);
  frag_pos = vec3(M * vec4(vert_pos.xyz, 1.0));
}
