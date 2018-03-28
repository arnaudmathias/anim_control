#include "animation.hpp"

Animation::Animation(std::string anim_name, double duration,
                     double ticks_per_second)
    : _name(anim_name),
      _animation_start(0.0f),
      _duration(duration),
      _ticks_per_second(ticks_per_second) {}

Animation::Animation(Animation const& src) { *this = src; }

Animation::~Animation(void) {}

Animation& Animation::operator=(Animation const& rhs) {
  if (this != &rhs) {
    channels = rhs.channels;
    _name = rhs._name;
    _animation_start = rhs._animation_start;
    _duration = rhs._duration;
    _ticks_per_second = rhs._ticks_per_second;
  }
  return (*this);
}

glm::mat4 Animation::animate(std::string key, float timestamp) {
  if (_animation_start == 0.0f || timestamp - _animation_start >= _duration) {
    _animation_start = timestamp;
  }
  float time_since_animation_start = timestamp - _animation_start;
  glm::mat4 res = glm::mat4(1.0f);
  auto channel_it = channels.find(key);
  if (channel_it == channels.end()) {
    return (res);
  }
  glm::vec3 interpolated_position = interpolatePosition(
      time_since_animation_start, channel_it->second.position_keys);
  glm::quat interpolated_rotation = interpolateRotation(
      time_since_animation_start, channel_it->second.rotation_keys);
  glm::vec3 interpolated_scaling = interpolateScaling(
      time_since_animation_start, channel_it->second.scale_keys);

  glm::mat4 mat_translation = glm::translate(interpolated_position);
  glm::mat4 mat_rotation = glm::mat4_cast(interpolated_rotation);
  glm::mat4 mat_scale = glm::scale(interpolated_scaling);

  res = mat_translation * mat_rotation * mat_scale;
  return (res);
}

bool Animation::addChannel(std::string key, const AnimChannel& channel) {
  auto res = channels.emplace(key, channel);
  if (res.second) {
    // TODO: ensure keys are sorted by timestamp
  }
  return (res.second);
}

glm::vec3 Animation::interpolatePosition(
    float time_in_seconds, const std::vector<AnimKey<glm::vec3>>& positions) {
  glm::vec3 position = glm::vec3(0.0f);

  if (positions.empty()) {
    return (position);
  }
  unsigned int index = nearest_index(time_in_seconds, positions);
  position = positions[index].value;
  return (position);
}

glm::quat Animation::interpolateRotation(
    float time_in_seconds, const std::vector<AnimKey<glm::quat>>& rotations) {
  glm::quat rotation = glm::quat();

  if (rotations.empty()) {
    return (rotation);
  }
  unsigned int index = nearest_index(time_in_seconds, rotations);
  rotation = rotations[index].value;
  return (rotation);
}

glm::vec3 Animation::interpolateScaling(
    float time_in_seconds, const std::vector<AnimKey<glm::vec3>>& scalings) {
  glm::vec3 scaling = glm::vec3(1.0f);

  if (scalings.empty()) {
    return (scaling);
  }
  unsigned int index = nearest_index(time_in_seconds, scalings);
  scaling = scalings[index].value;
  return (scaling);
}

template <typename T>
unsigned int Animation::nearest_index(float time_in_seconds,
                                      const std::vector<AnimKey<T>>& data) {
  if (data.size() == 0) {
    return (0);
  }
  for (unsigned int i = 0; i < data.size() - 1; i++) {
    if (time_in_seconds < data[i + 1].time) {
      return (i);
    }
  }
  return (0);
}
