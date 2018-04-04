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
  /*
  for (auto it = animations.begin(); it != animations.end(); it++) {
    delete it->second;
  }*/
  if (skeleton != nullptr) {
    delete skeleton;
  }
}

void Model::animate(float timestamp) {
  /*if (skeleton == nullptr) return;
  for (auto anim_it : animations) {
    Animation* anim = anim_it.second;
    for (auto node_it : skeleton->node_ids) {
      std::string node_name = node_it.first;
      unsigned short bone_index = node_it.second;
      skeleton->local_poses[bone_index] = anim->animate(node_name, timestamp);
    }
  }
  skeleton->local_to_global();
  attrib.bones.resize(skeleton->joint_count);
  for (unsigned short i = 0; i < skeleton->joint_count; i++) {
    attrib.bones[i] =
        global_inverse * skeleton->global_poses[i] * skeleton->offsets[i];
  }*/
}

void Model::updateAnimDebug(const render::Renderer& renderer) {
  if (skeleton == nullptr) return;
  _animAttrib.state.primitiveMode = render::PrimitiveMode::Lines;
  _animAttrib.state.depthTestFunc = render::DepthTestFunc::Always;
  _animAttrib.model = attrib.model;
  std::vector<glm::vec3> positions;
  for (unsigned short i = 0; i < skeleton->joint_count; i++) {
    const unsigned short parent_joint = skeleton->hierarchy[i];
    glm::mat4 bone_offset = global_inverse * skeleton->global_poses[i];
    glm::mat4 parent_bone_offset =
        global_inverse * skeleton->global_poses[parent_joint];

    glm::vec3 point_offset =
        glm::vec3(bone_offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::vec3 parent_point_offset =
        glm::vec3(parent_bone_offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    /* Experiment: draw bones as quad so we can control the thickness of lines
     * The quad is facing the camera, but the multiplication by the model matrix
     * in the shader give different thickness for each bones
     * FIXME
    glm::vec3 bone_dir = glm::normalize(parent_point_offset - point_offset);
    glm::vec3 bone_axis = glm::cross(
        bone_dir, glm::normalize(glm::vec3(-renderer.uniforms.view[2])));
    bone_axis =
        glm::vec3(glm::inverse(attrib.model) * glm::vec4(bone_axis, 1.0f));
    bone_axis = glm::normalize(bone_axis);
    bone_axis *= 0.01f;
    glm::vec3 p0 = point_offset - bone_axis;
    glm::vec3 p1 = point_offset + bone_axis;
    glm::vec3 p2 = parent_point_offset - bone_axis;
    glm::vec3 p3 = parent_point_offset + bone_axis;

    const std::vector<glm::vec3> quad = {{p0}, {p1}, {p2}, {p2}, {p3}, {p0}};
    positions.insert(positions.end(), quad.begin(), quad.end());*/

    positions.push_back(point_offset);
    positions.push_back(parent_point_offset);
  }
  if (_animAttrib.vaos.size() && _animAttrib.vaos[0] != nullptr) {
    _animAttrib.vaos[0]->update(positions);
  } else {
    _animAttrib.vaos.push_back(new VAO(positions));
  }
}
