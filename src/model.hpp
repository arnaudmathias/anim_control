#pragma once
#include <cstring>
#include <vector>
#include <unordered_map>
#include "anim.hpp"
#include "renderer.hpp"
#include "animation.hpp"

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
  unsigned short _joint_count;
};

class Mesh {
 public:
  Mesh(std::string key, std::vector<Vertex>& vertices,
       std::vector<unsigned int>& indices);
  Mesh(Mesh const& src);
  ~Mesh(void);
  Mesh& operator=(Mesh const& rhs);
  RenderAttrib renderAttrib;
  std::string key;

 private:
  Mesh(void);
};

struct Model {
  Model(void) = default;
  Model(Model const& src);
  Model& operator=(Model const& rhs);
  ~Model();

  std::vector<Mesh*> meshes;
  void pushRenderAttribs(Renderer& renderer);
  std::unordered_map<std::string, Animation*> animations;
  std::vector<glm::mat4> transforms;
  Skeleton* skeleton = nullptr;

 private:
  void animate(float time_in_second);
};
