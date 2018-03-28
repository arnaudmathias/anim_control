#include "model.hpp"

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
