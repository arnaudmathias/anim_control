#pragma once
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <cstring>
#include <iostream>
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

struct BoneInfo {
  glm::mat4 offset;
};

class MeshLoader {
 public:
  MeshLoader(void);
  MeshLoader(MeshLoader const& src);
  ~MeshLoader(void);
  MeshLoader& operator=(MeshLoader const& rhs);
  Model* loadScene(std::string filename);

 private:
  Model* current_model;
  std::unordered_map<std::string, unsigned int> bone_map;
  std::vector<BoneInfo> bones_info;
  void processChild(const aiScene* scene, const aiMesh* mesh,
                    unsigned int mesh_id);
  void processHierarchy(const aiScene* scene, aiNode* node,
                        unsigned short parent_id,
                        std::vector<unsigned short>& hierarchy);
  void processNode(const aiScene* scene, aiNode* node);
  Mesh* processMesh(const aiScene* scene, const aiMesh* mesh);
  void loadAnimations(const aiScene* scene);
  void loadBones(const aiScene* scene, const aiMesh* mesh);
  VertexBoneData getBoneData(unsigned int bone_id, float weight);
  void reset();
};
