#pragma once
#include <cstring>
#include <unordered_map>
#include <vector>
#include "anim.hpp"
#include "animation.hpp"
#include "renderer.hpp"

enum class ModelType { Static, Void, Animated };

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
  Model(enum ModelType type);
  Model(Model const& src);
  Model& operator=(Model const& rhs);
  ~Model();

  void update(float timestamp);
  void pushRenderAttribs(render::Renderer& renderer);
  void pushDebugRenderAttribs(render::Renderer& renderer);

  std::unordered_map<std::string, Animation*> animations;
  std::unordered_map<std::string, unsigned short> node_ids;
  render::Attrib attrib;
  Skeleton* skeleton = nullptr;
  glm::mat4 global_inverse;

 private:
  Model(void) = default;
  void animate(float time_in_second);
  void updateAnimDebug(const render::Renderer& renderer);
  enum ModelType _type;
  Shader* _debug_anim_shader = nullptr;
  Shader* _void_shader = nullptr;
  render::Attrib _animAttrib;
};
