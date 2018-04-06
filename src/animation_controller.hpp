#pragma once
#include <list>
#include <queue>
#include <string>
#include <vector>
#include "animation.hpp"
#include "model.hpp"

enum class AnimationState { Idle, Walking };

struct AState {
  AnimData* data = nullptr;
  enum AnimationState state = AnimationState::Idle;
  float weight = 0.0f;
  float animation_start = 0.0f;
};

class AnimationController {
 public:
  AnimationController(void);
  AnimationController(AnimationController const& src);
  ~AnimationController(void);
  AnimationController& operator=(AnimationController const& rhs);

  void changeAnimation(enum AnimationState state);
  void update(float t, Skeleton* skeleton,
              const std::unordered_map<std::string, AnimData>& animations);
  glm::mat4 blend(float t, std::string node_name);

 private:
  std::vector<AState> _states;
};
