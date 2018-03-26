#pragma once
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "anim.hpp"
#include "animation.hpp"
#include "renderer.hpp"

class Mesh {
 public:
  Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
  Mesh(Mesh const& src);
  ~Mesh(void);
  Mesh& operator=(Mesh const& rhs);
  RenderAttrib renderAttrib;

 private:
  Mesh(void);
};

struct Model {
  std::vector<Mesh*> meshes;
  void pushRenderAttribs(Renderer& renderer);
  std::unordered_map<std::string, Animation*> animations;
  ~Model();
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
  void processNode(const aiScene* scene, aiNode* node);
  Mesh* processMesh(const aiScene* scene, const aiMesh* mesh);
  void loadAnimations(const aiScene* scene);
  void loadBones(const aiScene* scene, const aiMesh* mesh);
  VertexBoneData getBoneData(unsigned int bone_id, float weight);
  void reset();
};
