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

Model* MeshLoader::loadModel(std::vector<glm::vec3> vertices) {
  reset();
  Model* model = new Model;
  VAO* vao = new VAO(vertices);
  model->renderAttrib.vaos.push_back(vao);
  model->renderAttrib.shader =
      new Shader("shaders/anim_debug.vert", "shaders/anim_debug.frag");
  return (model);
}

Model* MeshLoader::loadModel(std::vector<glm::vec3> vertices,
                             std::vector<unsigned int> indices) {
  reset();
  Model* model = new Model;
  VAO* vao = new VAO(vertices, indices);
  model->renderAttrib.vaos.push_back(vao);
  return (model);
}

void MeshLoader::parseNodeHierarchy(const aiScene* scene, aiNode* node,
                                    std::queue<unsigned short>& node_stack) {
  // Bind each node to an ID during DFS traversal
  if (scene->HasAnimations()) {
    if (bone_map.find(std::string(node->mName.C_Str())) != bone_map.end()) {
      node_map.emplace(std::string(node->mName.C_Str()), node_stack.size());
      node_stack.push(node_stack.size());
    }
  } else {
    node_map.emplace(std::string(node->mName.C_Str()), node_stack.size());
    node_stack.push(node_stack.size());
  }
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
  if (bone_map.find(std::string(node->mName.C_Str())) != bone_map.end()) {
    hierarchy.push_back(parent_id);
  }
  // hierarchy.push_back(parent_id);
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
    std::queue<unsigned short> node_queue;
    current_model->global_inverse = to_glm(scene->mRootNode->mTransformation);

    parseBoneHierarchy(scene, scene->mRootNode);
    parseNodeHierarchy(scene, scene->mRootNode, node_queue);
    current_model->node_ids = node_map;

    if (scene->HasAnimations()) {
      setupSkeleton(scene);
      loadAnimations(scene);
    }

  } else {
    std::cerr << "Can't load " << filename << std::endl;
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
    vertex.normal.x = mesh->mNormals[i].x;
    vertex.normal.y = mesh->mNormals[i].y;
    vertex.normal.z = mesh->mNormals[i].z;
    vertices.push_back(vertex);
  }
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int f = 0; f < face.mNumIndices; f++) {
      indices.push_back(face.mIndices[f]);
    }
  }
  populateVerticesBoneInfo(mesh, vertices);
  const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
  loadTextures(material, aiTextureType_DIFFUSE);
  loadTextures(material, aiTextureType_AMBIENT);
  loadTextures(material, aiTextureType_SPECULAR);

  VAO* vao = new VAO(vertices, indices);
  current_model->renderAttrib.vaos.push_back(vao);
}

void MeshLoader::loadMeshBones(const aiMesh* mesh) {
  for (unsigned int i = 0; i < mesh->mNumBones; i++) {
    std::string bone_name = std::string(mesh->mBones[i]->mName.C_Str());
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
      }
      unsigned int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
      float weight = mesh->mBones[i]->mWeights[j].mWeight;
      setBoneData(vertices[vertex_id], node_index, weight);
    }
  }
}

void MeshLoader::loadTextures(const aiMaterial* material, aiTextureType type) {
  TextureInfo tex_info = {};
  for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
    aiString str;
    material->GetTexture(type, i, &str);
    tex_info.texture = new Texture(str.C_Str());
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
    for (unsigned int j = 0; j < anim->mNumChannels; j++) {
      AnimChannel channel;
      const aiNodeAnim* node_anim = anim->mChannels[j];
      const std::string channel_key = std::string(node_anim->mNodeName.C_Str());

      // Copy positions
      for (unsigned k = 0; k < node_anim->mNumPositionKeys; k++) {
        AnimKey<glm::vec3> position_key = {
            node_anim->mPositionKeys[k].mTime,
            {node_anim->mPositionKeys[k].mValue.x,
             node_anim->mPositionKeys[k].mValue.y,
             node_anim->mPositionKeys[k].mValue.z}};
        channel.position_keys.push_back(position_key);
      }
      // Copy rotations
      for (unsigned k = 0; k < node_anim->mNumRotationKeys; k++) {
        AnimKey<glm::quat> rotation_key = {
            node_anim->mRotationKeys[k].mTime,
            {node_anim->mRotationKeys[k].mValue.w,
             node_anim->mRotationKeys[k].mValue.x,
             node_anim->mRotationKeys[k].mValue.y,
             node_anim->mRotationKeys[k].mValue.z}};
        channel.rotation_keys.push_back(rotation_key);
      }
      // Copy scaling
      for (unsigned k = 0; k < node_anim->mNumScalingKeys; k++) {
        AnimKey<glm::vec3> scaling_key = {
            node_anim->mScalingKeys[k].mTime,
            {node_anim->mScalingKeys[k].mValue.x,
             node_anim->mScalingKeys[k].mValue.y,
             node_anim->mScalingKeys[k].mValue.z}};
        channel.position_keys.push_back(scaling_key);
      }
      animation->addChannel(channel_key, channel);
    }
    current_model->animations.emplace(anim_name, animation);
  }
}

void MeshLoader::setupSkeleton(const aiScene* scene) {
  std::vector<unsigned short> hierarchy;
  processHierarchy(scene, scene->mRootNode, 0, hierarchy);

  current_model->skeleton = new Skeleton(hierarchy.size());
  for (unsigned int i = 0; i < hierarchy.size(); i++) {
    current_model->skeleton->_hierarchy[i] = hierarchy[i];
  }
  // Populate skeleton with bone offset
  for (auto node_it : node_map) {
    std::string node_name = node_it.first;
    unsigned short node_index = node_it.second;
    auto bone_it = bone_map.find(node_name);
    if (bone_it != bone_map.end()) {
      current_model->skeleton->_offsets[node_index] = bone_it->second.offset;
    }
  }
}

void MeshLoader::reset() {
  current_model = nullptr;
  bone_map.clear();
  node_map.clear();
}
