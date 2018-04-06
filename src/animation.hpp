#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "anim.hpp"

template <typename T>
struct AnimKey {
  double time;
  T value;
};

struct AnimChannel {
  std::vector<AnimKey<glm::vec3>> position_keys;
  std::vector<AnimKey<glm::quat>> rotation_keys;
  std::vector<AnimKey<glm::vec3>> scale_keys;
};

struct AnimData {
  std::string name;
  float duration_in_ticks;
  float ticks_per_second;
  std::unordered_map<std::string, AnimChannel> channels;
};

glm::mat4 animate(AnimData* data, std::string key, float timestamp);
float get_animation_time(float timestamp, float animation_start,
                         float ticks_per_second, float ticks_duration);
glm::vec3 interpolatePosition(float time_in_seconds,
                              const std::vector<AnimKey<glm::vec3>>& positions);
glm::quat interpolateRotation(float time_in_seconds,
                              const std::vector<AnimKey<glm::quat>>& rotations);
glm::vec3 interpolateScaling(float time_in_seconds,
                             const std::vector<AnimKey<glm::vec3>>& scalings);
template <typename T>
unsigned int nearest_index(float time_in_seconds,
                           const std::vector<AnimKey<T>>& data);
