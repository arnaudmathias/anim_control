#pragma once
#include <queue>

enum class AnimationState { Idle, Walking };

struct AState {
  enum AnimationState state = AnimationState::Idle;
  float weight = 0.0f;
};

struct AnimInfo {
  float _animation_start;
};

class AnimationController {
 public:
  AnimationController(void);
  AnimationController(AnimationController const& src);
  ~AnimationController(void);
  AnimationController& operator=(AnimationController const& rhs);

  void changeAnimation(enum AnimationState state);

 private:
  std::vector<AState> states;
};
