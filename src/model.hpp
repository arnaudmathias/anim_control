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
  unsigned short* hierarchy = nullptr;
  glm::mat4* local_poses = nullptr;
  glm::mat4* global_poses = nullptr;
  glm::mat4* offsets = nullptr;
  unsigned short joint_count = 0;
  glm::mat4 global_inverse = glm::mat4(1.0f);

  std::unordered_map<std::string, unsigned short> node_ids;
};

struct Model {
  Model(enum ModelType type);
  Model(Model const& src);
  Model& operator=(Model const& rhs);
  ~Model();

  std::unordered_map<std::string, AnimData> animations;
  render::Attrib attrib;
  Skeleton* skeleton = nullptr;

 private:
  Model(void) = default;
  void animate(float time_in_second);
  void updateAnimDebug(const render::Renderer& renderer);
  enum ModelType _type;
  render::Attrib _animAttrib;
};
