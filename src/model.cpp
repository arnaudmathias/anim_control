#include "model.hpp"

Skeleton::Skeleton(unsigned short joint_count) : joint_count(joint_count) {
  hierarchy = new unsigned short[joint_count];
  local_poses = new glm::mat4[joint_count];
  global_poses = new glm::mat4[joint_count];
  offsets = new glm::mat4[joint_count];
  std::memset(hierarchy, 0, sizeof(*hierarchy) * joint_count);
  for (unsigned short i = 0; i < joint_count; i++) {
    local_poses[i] = glm::mat4(0.0f);
    global_poses[i] = glm::mat4(0.0f);
    offsets[i] = glm::mat4(0.0f);
  }
}

Skeleton::Skeleton(Skeleton const& src) { *this = src; }

Skeleton::~Skeleton(void) {
  delete[] hierarchy;
  delete[] local_poses;
  delete[] global_poses;
  delete[] offsets;
}

Skeleton& Skeleton::operator=(Skeleton const& rhs) {
  if (this != &rhs) {
    joint_count = rhs.joint_count;
    hierarchy = new unsigned short[joint_count];
    local_poses = new glm::mat4[joint_count];
    global_poses = new glm::mat4[joint_count];
    offsets = new glm::mat4[joint_count];
    std::memcpy(hierarchy, rhs.hierarchy, sizeof(*hierarchy) * joint_count);
    std::memcpy(local_poses, rhs.local_poses,
                sizeof(*local_poses) * joint_count);
    std::memcpy(global_poses, rhs.global_poses,
                sizeof(*global_poses) * joint_count);
    std::memcpy(offsets, rhs.offsets, sizeof(*offsets) * joint_count);
    node_ids = rhs.node_ids;
    global_inverse = rhs.global_inverse;
  }
  return (*this);
}

void Skeleton::local_to_global() {
  global_poses[0] = local_poses[0];
  for (unsigned short i = 1; i < joint_count; i++) {
    const unsigned short parent_joint = hierarchy[i];
    global_poses[i] = global_poses[parent_joint] * local_poses[i];
  }
}

Model::Model(enum ModelType type) : _type(type) {
  if (_type == ModelType::Animated) {
    attrib.shader_key = "anim";
    _animAttrib.shader_key = "anim_debug";
  } else if (_type == ModelType::Void) {
    attrib.shader_key = "void_model";
  }
}

Model::Model(Model const& rhs) { *this = rhs; }

Model& Model::operator=(Model const& rhs) {
  if (this != &rhs) {
    skeleton = rhs.skeleton != nullptr ? new Skeleton(*rhs.skeleton) : nullptr;
  }
  return (*this);
}

Model::~Model(void) {
  for (auto& vao : attrib.vaos) {
    delete vao;
  }
  for (auto& vao : _animAttrib.vaos) {
    delete vao;
  }
  if (skeleton != nullptr) {
    delete skeleton;
  }
}
