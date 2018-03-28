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
#include "model.hpp"

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
