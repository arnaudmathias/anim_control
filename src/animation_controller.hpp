#pragma once
#include <list>
#include <queue>
#include <string>
#include <vector>
#include "animation.hpp"
#include "model.hpp"

enum class AnimNodeState { Increase, Decrease, Constant };

struct AState {
  AnimData* data = nullptr;
  float weight = 1.0f;
  float blend_speed = 0.5f;
  float animation_start = 0.0f;
  AnimNodeState node_state;
};

class AnimationController {
 public:
  AnimationController(void);
  AnimationController(AnimationController const& src);
  ~AnimationController(void);
  AnimationController& operator=(AnimationController const& rhs);

  void changeAnimation(float t, AnimData* data, float start_weight,
                       AnimNodeState node_state);
  void update(float t, Skeleton* skeleton);
  glm::mat4 blend(float t, std::string node_name);

 private:
  std::vector<AState> _states;
};
