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
  void addChannel(std::string key, const AnimChannel& channel);

  std::unordered_map<std::string, AnimChannel> channels;

 private:
  Animation(void) = default;

  std::string _name;
  double _duration;  // in ticks
  double _ticks_per_second;
};
