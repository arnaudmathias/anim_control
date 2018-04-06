#include "animation_controller.hpp"

std::string animations_state_files[2] = {"anims/Idle.dae", "anims/Walking.dae"};

AnimationController::AnimationController(void) {}

AnimationController::AnimationController(AnimationController const& src) {
  *this = src;
}

AnimationController::~AnimationController(void) {}

AnimationController& AnimationController::operator=(
    AnimationController const& rhs) {
  if (this != &rhs) {
  }
  return (*this);
}

void AnimationController::changeAnimation(enum AnimationState state) {
  AState new_state = {};
  new_state.state = state;
  new_state.weight = 1.0f;
}

void AnimationController::update(
    float t, Skeleton* skeleton,
    const std::unordered_map<std::string, AnimData>& animations) {
  if (skeleton == nullptr) {
    return;
  }
  for (auto node_it : skeleton->node_ids) {
    std::string node_name = node_it.first;
    unsigned short bone_index = node_it.second;
    skeleton->local_poses[bone_index] = blend(t, node_name);
  }
}

glm::mat4 AnimationController::blend(float t, std::string node_name) {
  glm::mat4 res = glm::mat4(1.0f);
  if (_states.empty()) {
    return (res);
  }
  std::vector<glm::vec3> positions(_states.size());
  std::vector<glm::quat> rotations(_states.size());
  std::vector<glm::vec3> scalings(_states.size());
  std::vector<float> atimes(_states.size());
  for (unsigned int i = 0; i < _states.size(); i++) {
    if (_states[i].data != nullptr) {
      float anim_time = get_animation_time(t, _states[i].animation_start,
                                           _states[i].data->ticks_per_second,
                                           _states[i].data->duration_in_ticks);
      atimes[i] = anim_time;
    } else {
      atimes[i] = 0.0f;
    }
  }

  for (unsigned int i = 0; i < _states.size(); i++) {
    if (_states[i].data == nullptr || _states[i].weight == 0.0f) continue;
    auto it = _states[i].data->channels.find(node_name);
    if (it == _states[i].data->channels.end()) {
      continue;
    }
    positions[i] = interpolatePosition(atimes[i], it->second.position_keys);
    rotations[i] = interpolateRotation(atimes[i], it->second.rotation_keys);
    scalings[i] = interpolateScaling(atimes[i], it->second.scale_keys);
  }
  glm::vec3 interpolated_position = positions[0];
  glm::quat interpolated_rotation = rotations[0];
  glm::vec3 interpolated_scaling = scalings[0];
  for (unsigned int i = 1; i < _states.size(); i++) {
    // TODO: use actual node weight
    interpolated_position = glm::mix(interpolated_position, positions[i], 0.5f);
    interpolated_rotation =
        glm::slerp(interpolated_rotation, rotations[i], 0.5f);
    interpolated_scaling = glm::mix(interpolated_scaling, scalings[i], 0.5f);
  }

  glm::mat4 mat_translation = glm::translate(interpolated_position);
  glm::mat4 mat_rotation = glm::mat4_cast(interpolated_rotation);
  glm::mat4 mat_scale = glm::scale(interpolated_scaling);
  res = mat_translation * mat_rotation * mat_scale;
  return (res);
}
