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

class Animation {
 public:
  Animation(std::string anim_name, double duration, double ticks_per_second);
  Animation(Animation const& src);
  ~Animation(void);
  Animation& operator=(Animation const& rhs);
  bool addChannel(std::string key, const AnimChannel& channel);
  glm::mat4 animate(std::string key, float timestamp);

  std::unordered_map<std::string, AnimChannel> channels;

 private:
  Animation(void) = default;
  glm::vec3 interpolatePosition(
      float time_in_seconds, const std::vector<AnimKey<glm::vec3>>& positions);
  glm::quat interpolateRotation(
      float time_in_seconds, const std::vector<AnimKey<glm::quat>>& rotations);
  glm::vec3 interpolateScaling(float time_in_seconds,
                               const std::vector<AnimKey<glm::vec3>>& scalings);

  template <typename T>
  unsigned int nearest_index(float time_in_seconds,
                             const std::vector<AnimKey<T>>& data);

  std::string _name;
  float _animation_start;
  double _duration;  // in ticks
  double _ticks_per_second;
};
