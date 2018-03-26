#include "loader.hpp"

Mesh::Mesh(void) {}

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
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
  }
  return (*this);
}

void Model::pushRenderAttribs(Renderer& renderer) {
  for (const auto& mesh : meshes) {
    renderer.addRenderAttrib(mesh->renderAttrib);
  }
}

Model::~Model(void) {
  for (auto& mesh : meshes) {
    delete mesh;
  }
}

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
    glm::mat4 globalInverse = to_glm(scene->mRootNode->mTransformation);
    globalInverse = glm::inverse(globalInverse);

    processNode(scene, scene->mRootNode);

    std::vector<VertexBoneData> bones;

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
  Mesh* load_mesh = new Mesh(vertices, indices);
  return (load_mesh);
}

void MeshLoader::loadBones(const aiScene* scene, const aiMesh* mesh) {
  std::vector<VertexBoneData> bones;
  bones.resize(mesh->mNumVertices);
  for (unsigned int i = 0; i < mesh->mNumBones; i++) {
    unsigned int bone_index;
    std::string bone_name = std::string(mesh->mBones[i]->mName.C_Str());
    std::cout << "bone_name: " << bone_name << std::endl;
    auto bone_it = bone_map.find(bone_name);
    if (bone_it == bone_map.end()) {
      bone_index = bones_info.size();
      struct BoneInfo bone_info = {};
      bones_info.push_back(bone_info);
      bone_map[bone_name] = bone_index;
    } else {
      std::cout << "bone_name already loaded" << std::endl;
      bone_index = bone_it->second;
    }
    bones_info[bone_index].offset = to_glm(mesh->mBones[i]->mOffsetMatrix);
    for (uint j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
      unsigned int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
      float weight = mesh->mBones[i]->mWeights[j].mWeight;
      bones[vertex_id] = getBoneData(bone_index, weight);
    }
  }
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
