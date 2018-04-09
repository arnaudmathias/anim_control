#include "animation_controller.hpp"

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

void AnimationController::changeAnimation(AState state) {
  if (_states.empty() == false && _states.end()->data == state.data) {
    // This animation is already playing, we don't need to push a new state
    return;
  }
  for (auto& st : _states) {
    // Invalide every states already running
    st.node_state = AnimNodeState::Decrease;
  }
  _states.push_back(state);
}

void AnimationController::update(float t, Skeleton* skeleton) {
  if (skeleton == nullptr) {
    return;
  }
  for (auto& state : _states) {
    if (state.node_state == AnimNodeState::Increase) {
      state.weight += 0.01f;
    }
    if (state.node_state == AnimNodeState::Decrease) {
      state.weight -= 0.01f;
    }
    if (state.weight > 1.0f) {
      state.weight = 1.0f;
      state.node_state = AnimNodeState::Constant;
    }
    if (state.weight < 0.0f) {
      state.weight = 0.0f;
      state.node_state = AnimNodeState::Constant;
    }
  }
  _states.erase(std::remove_if(_states.begin(), _states.end(),
                               [](AState& state) {
                                 if (state.weight <= 0.0f) {
                                   return (true);
                                 }
                                 return (false);
                               }),
                _states.end());
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
    float weight = _states[i].weight;
    interpolated_position =
        glm::mix(interpolated_position, positions[i], weight);
    interpolated_rotation =
        glm::slerp(interpolated_rotation, rotations[i], weight);
    interpolated_scaling = glm::mix(interpolated_scaling, scalings[i], weight);
  }

  glm::mat4 mat_translation = glm::translate(interpolated_position);
  glm::mat4 mat_rotation = glm::mat4_cast(interpolated_rotation);
  glm::mat4 mat_scale = glm::scale(interpolated_scaling);
  res = mat_translation * mat_rotation * mat_scale;
  return (res);
}
