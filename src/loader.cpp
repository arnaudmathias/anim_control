#include "loader.hpp"

MeshLoader::MeshLoader(void) {}

MeshLoader::MeshLoader(MeshLoader const& src) { *this = src; }

MeshLoader::~MeshLoader(void) {}

MeshLoader& MeshLoader::operator=(MeshLoader const& rhs) {
  if (this != &rhs) {
  }
  return (*this);
}

glm::mat4 to_glm(aiMatrix4x4t<float> aimat) {
  glm::mat4 mat(0.0f);
  float* mat_array = glm::value_ptr(mat);
  for (unsigned int i = 0; i < 16; i++) {
    mat_array[i] = aimat[i / 4][i % 4];
  }
  mat = glm::transpose(mat);
  return (mat);
}

void MeshLoader::parseNodeHierarchy(const aiScene* scene, aiNode* node,
                                    std::queue<unsigned short>& node_stack) {
  // Bind each node to an ID during DFS traversal
  node_map.emplace(std::string(node->mName.C_Str()), node_stack.size());
  node_stack.push(node_stack.size());
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    processMesh(scene, mesh);
  }
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    parseNodeHierarchy(scene, node->mChildren[i], node_stack);
  }
}

void MeshLoader::parseBoneHierarchy(const aiScene* scene, aiNode* node) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    if (mesh->HasBones()) {
      loadMeshBones(mesh);
    }
  }
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    parseBoneHierarchy(scene, node->mChildren[i]);
  }
}

void MeshLoader::processHierarchy(const aiScene* scene, aiNode* node,
                                  unsigned short parent_id,
                                  std::vector<unsigned short>& hierarchy) {
  // flatten skeleton hierarchy by DFS traversal
  unsigned short id = hierarchy.size();
  hierarchy.push_back(parent_id);
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processHierarchy(scene, node->mChildren[i], id, hierarchy);
  }
}

Model* MeshLoader::loadScene(std::string filename) {
  reset();
  Model* model = new Model;
  current_model = model;
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(
      filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                    aiProcess_FlipUVs);
  if (scene) {
    std::cout << "scene num anim:" << scene->mNumAnimations << std::endl;
    std::cout << "scene num mesh:" << scene->mNumMeshes << std::endl;
    glm::mat4 globalInverse = to_glm(scene->mRootNode->mTransformation);
    globalInverse = glm::inverse(globalInverse);

    parseBoneHierarchy(scene, scene->mRootNode);
    parseNodeHierarchy(scene, scene->mRootNode, std::queue<unsigned short>());
    current_model->node_ids = node_map;

    std::vector<VertexBoneData> bones;
    if (scene->HasAnimations()) {
      std::vector<unsigned short> hierarchy;
      processHierarchy(scene, scene->mRootNode, 0, hierarchy);
      std::cout << "hierarchy: " << hierarchy.size() << std::endl;

      current_model->skeleton = new Skeleton(hierarchy.size());
      for (unsigned int i = 0; i < hierarchy.size(); i++) {
        // std::cout << "hierarchy[" << i << "] " << hierarchy[i] << std::endl;
        current_model->skeleton->_hierarchy[i] = hierarchy[i];
      }
      loadAnimations(scene);
    }

  } else {
    std::cout << "Can't load " << filename << std::endl;
  }
  current_model = nullptr;
  importer.FreeScene();
  return (model);
}

void MeshLoader::processMesh(const aiScene* scene, const aiMesh* mesh) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::string mesh_name = std::string(mesh->mName.C_Str());
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex = {};
    vertex.position.x = mesh->mVertices[i].x;
    vertex.position.y = mesh->mVertices[i].y;
    vertex.position.z = mesh->mVertices[i].z;

    vertices.push_back(vertex);
  }
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int f = 0; f < face.mNumIndices; f++) {
      indices.push_back(face.mIndices[f]);
    }
  }
  populateVerticesBoneInfo(mesh, vertices);
  std::cout << "vertices: " << vertices.size() << std::endl;
  std::cout << "indices: " << indices.size() << std::endl;
  VAO* vao = new VAO(vertices, indices);
  current_model->renderAttrib.vaos.push_back(vao);
}

void MeshLoader::loadMeshBones(const aiMesh* mesh) {
  for (unsigned int i = 0; i < mesh->mNumBones; i++) {
    std::string bone_name = std::string(mesh->mBones[i]->mName.C_Str());
    // std::cout << "bone_name: " << bone_name << std::endl;
    auto bone_it = bone_map.find(bone_name);
    if (bone_it == bone_map.end()) {
      struct BoneInfo bone_info = {};
      bone_it = bone_map.emplace(bone_name, bone_info).first;
    }
    bone_it->second.offset = to_glm(mesh->mBones[i]->mOffsetMatrix);
  }
}

void MeshLoader::populateVerticesBoneInfo(const aiMesh* mesh,
                                          std::vector<Vertex>& vertices) {
  for (unsigned int i = 0; i < mesh->mNumBones; i++) {
    std::string bone_name = std::string(mesh->mBones[i]->mName.C_Str());
    for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
      unsigned int node_index = 0;
      auto node_it = node_map.find(bone_name);
      if (node_it != node_map.end()) {
        node_index = static_cast<unsigned int>(node_it->second);
      } else {
        std::cout << "invalid node_index" << std::endl;
      }
      unsigned int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
      float weight = mesh->mBones[i]->mWeights[j].mWeight;
      setBoneData(vertices[vertex_id], node_index, weight);
    }
  }
}

void MeshLoader::setBoneData(Vertex& vertex, unsigned int bone_id,
                             float weight) {
  for (unsigned int i = 0; i < 4; i++) {
    if (vertex.weights[i] == 0.0f) {
      vertex.bone_ids[i] = bone_id;
      vertex.weights[i] = weight;
      break;
    }
  }
}

void MeshLoader::loadAnimations(const aiScene* scene) {
  for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
    const aiAnimation* anim = scene->mAnimations[i];
    std::string anim_name = std::string(anim->mName.C_Str());
    if (anim_name.empty()) {
      anim_name = std::to_string(i);
    }
    Animation* animation =
        new Animation(anim_name, anim->mDuration, anim->mTicksPerSecond);
    std::cout << "animations_name: " << anim_name << std::endl;
    for (unsigned int j = 0; j < anim->mNumChannels; j++) {
      AnimChannel channel;
      const aiNodeAnim* node_anim = anim->mChannels[j];
      const std::string channel_key = std::string(node_anim->mNodeName.C_Str());
      // std::cout << "node key: " << channel_key << std::endl;

      // Copy positions
      for (unsigned k = 0; k < node_anim->mNumPositionKeys; k++) {
        AnimKey<glm::vec3> position_key = {};
        position_key.time = node_anim->mPositionKeys[k].mTime;
        position_key.value.x = node_anim->mPositionKeys[k].mValue.x;
        position_key.value.y = node_anim->mPositionKeys[k].mValue.y;
        position_key.value.z = node_anim->mPositionKeys[k].mValue.z;
        channel.position_keys.push_back(position_key);
      }
      // Copy rotations
      for (unsigned k = 0; k < node_anim->mNumRotationKeys; k++) {
        AnimKey<glm::quat> rotation_key = {};
        rotation_key.time = node_anim->mRotationKeys[k].mTime;
        rotation_key.value.x = node_anim->mRotationKeys[k].mValue.x;
        rotation_key.value.y = node_anim->mRotationKeys[k].mValue.y;
        rotation_key.value.z = node_anim->mRotationKeys[k].mValue.z;
        rotation_key.value.w = node_anim->mRotationKeys[k].mValue.w;
        channel.rotation_keys.push_back(rotation_key);
      }
      // Copy scaling
      for (unsigned k = 0; k < node_anim->mNumScalingKeys; k++) {
        AnimKey<glm::vec3> scaling_key = {};
        scaling_key.time = node_anim->mScalingKeys[k].mTime;
        scaling_key.value.x = node_anim->mScalingKeys[k].mValue.x;
        scaling_key.value.y = node_anim->mScalingKeys[k].mValue.y;
        scaling_key.value.z = node_anim->mScalingKeys[k].mValue.z;
        channel.position_keys.push_back(scaling_key);
      }
      animation->addChannel(channel_key, channel);
    }
    current_model->animations.emplace(anim_name, animation);
  }
}

void MeshLoader::reset() {
  current_model = nullptr;
  bone_map.clear();
  node_map.clear();
}
