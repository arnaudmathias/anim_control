#include "loader.hpp"

Skeleton::Skeleton(unsigned short joint_count) : _joint_count(joint_count) {
  _hierarchy = new unsigned short[_joint_count];
  _local_poses = new glm::mat4[_joint_count];
  _global_poses = new glm::mat4[_joint_count];
}

Skeleton::Skeleton(Skeleton const& src) { *this = src; }

Skeleton::~Skeleton(void) {
  delete[] _hierarchy;
  delete[] _local_poses;
  delete[] _global_poses;
}

Skeleton& Skeleton::operator=(Skeleton const& rhs) {
  if (this != &rhs) {
    _joint_count = rhs._joint_count;
    _hierarchy = new unsigned short[_joint_count];
    _local_poses = new glm::mat4[_joint_count];
    _global_poses = new glm::mat4[_joint_count];
    std::memcpy(_hierarchy, rhs._hierarchy, sizeof(*_hierarchy));
    std::memcpy(_local_poses, rhs._local_poses, sizeof(*_local_poses));
    std::memcpy(_global_poses, rhs._global_poses, sizeof(*_global_poses));
  }
  return (*this);
}

void Skeleton::local_to_global() {
  _global_poses[0] = _local_poses[0];
  for (unsigned short i = 1; i < _joint_count; i++) {
    const unsigned short parent_joint = _hierarchy[i];
    _global_poses[i] = _global_poses[parent_joint] * _local_poses[i];
  }
}

Mesh::Mesh(void) {}

Mesh::Mesh(std::string key, std::vector<Vertex>& vertices,
           std::vector<unsigned int>& indices)
    : key(key) {
  renderAttrib.vaos.push_back(new VAO(vertices, indices));
}

Mesh::Mesh(Mesh const& src) { *this = src; }

Mesh::~Mesh(void) {
  for (auto& vao : renderAttrib.vaos) {
    delete vao;
  }
}

Mesh& Mesh::operator=(Mesh const& rhs) {
  if (this != &rhs) {
    key = rhs.key;
  }
  return (*this);
}

Model::Model(Model const& rhs) { *this = rhs; }

Model& Model::operator=(Model const& rhs) {
  if (this != &rhs) {
    for (auto& mesh : rhs.meshes) {
      meshes.push_back(new Mesh(*mesh));
    }
    for (auto& anim : rhs.animations) {
      animations.emplace(anim.first, new Animation(*anim.second));
    }
    skeleton = rhs.skeleton != nullptr ? new Skeleton(*rhs.skeleton) : nullptr;
  }
  return (*this);
}

Model::~Model(void) {
  for (auto& mesh : meshes) {
    delete mesh;
  }
  for (auto it = animations.begin(); it != animations.end(); it++) {
    delete it->second;
  }
  if (skeleton != nullptr) {
    delete skeleton;
  }
}

void Model::pushRenderAttribs(Renderer& renderer) {
  for (const auto& mesh : meshes) {
    renderer.addRenderAttrib(mesh->renderAttrib);
  }
}

void Model::animate(float time_in_seconds) {}

MeshLoader::MeshLoader(void) {}

MeshLoader::MeshLoader(MeshLoader const& src) { *this = src; }

MeshLoader::~MeshLoader(void) {}

MeshLoader& MeshLoader::operator=(MeshLoader const& rhs) {
  if (this != &rhs) {
  }
  return (*this);
}

glm::mat4 to_glm(aiMatrix4x4t<float> aimat) {
  glm::mat4 mat;
  std::memcpy(glm::value_ptr(mat), &aimat.a1, sizeof(aimat));
  mat = glm::transpose(mat);
  return (mat);
}

void MeshLoader::processNode(const aiScene* scene, aiNode* node) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    current_model->meshes.push_back(processMesh(scene, mesh));
  }
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(scene, node->mChildren[i]);
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

    processNode(scene, scene->mRootNode);
    std::cout << "models: " << current_model->meshes.size() << std::endl;

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
  std::cout << "model: " << model->meshes.size() << std::endl;
  importer.FreeScene();
  return (model);
}

Mesh* MeshLoader::processMesh(const aiScene* scene, const aiMesh* mesh) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::string mesh_name = std::string(mesh->mName.C_Str());
  if (mesh->HasBones()) {
    loadBones(scene, mesh);
  }

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
  std::cout << "vertices: " << vertices.size() << std::endl;
  std::cout << "indices: " << indices.size() << std::endl;
  Mesh* load_mesh = new Mesh(mesh_name, vertices, indices);
  return (load_mesh);
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

void MeshLoader::loadBones(const aiScene* scene, const aiMesh* mesh) {
  std::vector<VertexBoneData> bones;
  bones.resize(mesh->mNumVertices);
  std::cout << "mesh->mNumBones: " << mesh->mNumBones << std::endl;
  for (unsigned int i = 0; i < mesh->mNumBones; i++) {
    unsigned int bone_index;
    std::string bone_name = std::string(mesh->mBones[i]->mName.C_Str());
    // std::cout << "bone_name: " << bone_name << std::endl;
    auto bone_it = bone_map.find(bone_name);
    if (bone_it == bone_map.end()) {
      bone_index = bones_info.size();
      struct BoneInfo bone_info = {};
      bones_info.push_back(bone_info);
      bone_map[bone_name] = bone_index;
    } else {
      // std::cout << "bone_name already loaded" << std::endl;
      bone_index = bone_it->second;
    }
    bones_info[bone_index].offset = to_glm(mesh->mBones[i]->mOffsetMatrix);
    for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
      unsigned int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
      float weight = mesh->mBones[i]->mWeights[j].mWeight;
      bones[vertex_id] = getBoneData(bone_index, weight);
    }
  }
  std::cout << "bones count: " << bone_map.size() << std::endl;
}

VertexBoneData MeshLoader::getBoneData(unsigned int bone_id, float weight) {
  VertexBoneData bone_data;
  for (unsigned int i = 0; i < 4; i++) {
    if (weight == 0.0f) {
      bone_data.bone_ids[i] = bone_id;
      bone_data.weights[i] = weight;
      break;
    }
  }
  return (bone_data);
}

void MeshLoader::reset() { current_model = nullptr; }
