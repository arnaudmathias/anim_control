#include "animation.hpp"

glm::mat4 animate(AnimData* data, std::string key, float timestamp) {
  float time_since_animation_start = timestamp - 0.0f;
  float time_in_tick = time_since_animation_start * data->ticks_per_second;
  float animation_time = fmod(time_in_tick, data->duration_in_ticks);
  glm::mat4 res = glm::mat4(1.0f);
  auto channel_it = data->channels.find(key);
  if (channel_it == data->channels.end()) {
    return (res);
  }
  glm::vec3 interpolated_position =
      interpolatePosition(animation_time, channel_it->second.position_keys);
  glm::quat interpolated_rotation =
      interpolateRotation(animation_time, channel_it->second.rotation_keys);
  glm::vec3 interpolated_scaling =
      interpolateScaling(animation_time, channel_it->second.scale_keys);

  glm::mat4 mat_translation = glm::translate(interpolated_position);
  glm::mat4 mat_rotation = glm::mat4_cast(interpolated_rotation);
  glm::mat4 mat_scale = glm::scale(interpolated_scaling);

  res = mat_translation * mat_rotation * mat_scale;
  return (res);
}
glm::vec3 interpolatePosition(
    float time_in_seconds, const std::vector<AnimKey<glm::vec3>>& positions) {
  glm::vec3 position = glm::vec3(0.0f);

  if (positions.empty()) {
    return (position);
  }
  unsigned int index = nearest_index(time_in_seconds, positions);
  /*float deltaTime = positions[index + 1].time - positions[index].time;
  float inter_factor = (positions[index].time - time_in_seconds) / deltaTime;
  position = glm::mix(positions[index].value, positions[index + 1].value,
                      inter_factor);*/
  position = positions[index].value;
  return (position);
}

glm::quat interpolateRotation(
    float time_in_seconds, const std::vector<AnimKey<glm::quat>>& rotations) {
  glm::quat rotation = glm::quat();

  if (rotations.empty()) {
    return (rotation);
  }
  unsigned int index = nearest_index(time_in_seconds, rotations);
  rotation = rotations[index].value;
  return (rotation);
}

glm::vec3 interpolateScaling(float time_in_seconds,
                             const std::vector<AnimKey<glm::vec3>>& scalings) {
  glm::vec3 scaling = glm::vec3(1.0f);

  if (scalings.empty()) {
    return (scaling);
  }
  unsigned int index = nearest_index(time_in_seconds, scalings);
  scaling = scalings[index].value;
  return (scaling);
}

template <typename T>
unsigned int nearest_index(float time_in_seconds,
                           const std::vector<AnimKey<T>>& data) {
  if (data.size() == 0) {
    return (0);
  }
  unsigned int i;
  for (i = 0; i < data.size() - 1; i++) {
    if (time_in_seconds < data[i + 1].time) {
      return (i);
    }
  }
  return (i);
}
