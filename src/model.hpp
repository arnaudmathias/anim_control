#pragma once
#include <cstring>
#include <unordered_map>
#include <vector>
#include "anim.hpp"
#include "animation.hpp"
#include "renderer.hpp"

class Skeleton {
 public:
  Skeleton(unsigned short joint_count);
  Skeleton(Skeleton const& src);
  ~Skeleton(void);
  Skeleton& operator=(Skeleton const& rhs);
  void local_to_global();

  Skeleton(void) = default;
  unsigned short* _hierarchy = nullptr;
  glm::mat4* _local_poses = nullptr;
  glm::mat4* _global_poses = nullptr;
  glm::mat4* _offsets = nullptr;
  unsigned short _joint_count;
};

struct Model {
  Model(void);
  Model(Model const& src);
  Model& operator=(Model const& rhs);
  ~Model();

  void update(float timestamp);
  void pushRenderAttribs(Renderer& renderer);
  void pushDebugRenderAttribs(Renderer& renderer);

  std::unordered_map<std::string, Animation*> animations;
  std::unordered_map<std::string, unsigned short> node_ids;
  RenderAttrib renderAttrib;
  Skeleton* skeleton = nullptr;
  glm::mat4 global_inverse;

 private:
  void animate(float time_in_second);
  void updateAnimDebug();
  Shader* _debug_anim_shader = nullptr;
  RenderAttrib _animRenderAttrib;
};
