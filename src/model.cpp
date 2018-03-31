#include "model.hpp"

Skeleton::Skeleton(unsigned short joint_count) : _joint_count(joint_count) {
  _hierarchy = new unsigned short[_joint_count];
  _local_poses = new glm::mat4[_joint_count];
  _global_poses = new glm::mat4[_joint_count];
  _offsets = new glm::mat4[_joint_count];
  std::memset(_hierarchy, 0, sizeof(*_hierarchy) * _joint_count);
  for (unsigned short i = 0; i < _joint_count; i++) {
    _local_poses[i] = glm::mat4(0.0f);
    _global_poses[i] = glm::mat4(0.0f);
    _offsets[i] = glm::mat4(0.0f);
  }
}

Skeleton::Skeleton(Skeleton const& src) { *this = src; }

Skeleton::~Skeleton(void) {
  delete[] _hierarchy;
  delete[] _local_poses;
  delete[] _global_poses;
  delete[] _offsets;
}

Skeleton& Skeleton::operator=(Skeleton const& rhs) {
  if (this != &rhs) {
    _joint_count = rhs._joint_count;
    _hierarchy = new unsigned short[_joint_count];
    _local_poses = new glm::mat4[_joint_count];
    _global_poses = new glm::mat4[_joint_count];
    _offsets = new glm::mat4[_joint_count];
    std::memcpy(_hierarchy, rhs._hierarchy, sizeof(*_hierarchy) * _joint_count);
    std::memcpy(_local_poses, rhs._local_poses,
                sizeof(*_local_poses) * _joint_count);
    std::memcpy(_global_poses, rhs._global_poses,
                sizeof(*_global_poses) * _joint_count);
    std::memcpy(_offsets, rhs._offsets, sizeof(*_offsets) * _joint_count);
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

Model::Model(void) {
  _debug_anim_shader =
      new Shader("shaders/anim_debug.vert", "shaders/anim_debug.frag");
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
  for (auto it = animations.begin(); it != animations.end(); it++) {
    delete it->second;
  }
  if (skeleton != nullptr) {
    delete skeleton;
  }
}

void Model::update(float timestamp) {
  glm::mat4 mat_translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
  glm::mat4 mat_rotation =
      glm::eulerAngleYXZ(glm::radians(90.0f), glm::radians(0.0f), 0.0f);
  glm::mat4 mat_scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
  attrib.model = mat_translation * mat_rotation * mat_scale;
  animate(timestamp);
}

void Model::pushRenderAttribs(render::Renderer& renderer) {
  renderer.addAttrib(attrib);
}

void Model::pushDebugRenderAttribs(render::Renderer& renderer) {
  updateAnimDebug();
  renderer.addAttrib(_animAttrib);
}

void Model::animate(float timestamp) {
  if (skeleton == nullptr) return;
  for (auto anim_it : animations) {
    Animation* anim = anim_it.second;
    for (auto node_it : node_ids) {
      std::string node_name = node_it.first;
      unsigned short bone_index = node_it.second;
      skeleton->_local_poses[bone_index] = anim->animate(node_name, timestamp);
    }
  }
  skeleton->local_to_global();
  attrib.bones.resize(skeleton->_joint_count);
  for (unsigned short i = 0; i < skeleton->_joint_count; i++) {
    attrib.bones[i] =
        global_inverse * skeleton->_global_poses[i] * skeleton->_offsets[i];
  }
}

void Model::updateAnimDebug() {
  if (skeleton == nullptr) return;
  _animAttrib.state.primitiveMode = render::PrimitiveMode::Lines;
  _animAttrib.state.depthTestFunc = render::DepthTestFunc::Always;
  _animAttrib.model = attrib.model;
  _animAttrib.shader = _debug_anim_shader;
  std::vector<glm::vec3> positions;
  for (unsigned short i = 0; i < skeleton->_joint_count; i++) {
    const unsigned short parent_joint = skeleton->_hierarchy[i];
    glm::mat4 bone_offset = global_inverse * skeleton->_global_poses[i];
    glm::mat4 parent_bone_offset =
        global_inverse * skeleton->_global_poses[parent_joint];

    glm::vec3 point_offset =
        glm::vec3(bone_offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::vec3 parent_point_offset =
        glm::vec3(parent_bone_offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    positions.push_back(point_offset);
    positions.push_back(parent_point_offset);
  }
  if (_animAttrib.vaos.size() && _animAttrib.vaos[0] != nullptr) {
    _animAttrib.vaos[0]->update(positions);
  } else {
    _animAttrib.vaos.push_back(new VAO(positions));
  }
}
