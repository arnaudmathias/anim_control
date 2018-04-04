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

void AnimationController::changeAnimation(enum AnimationState state) {
  AState new_state = {};
  new_state.state = state;
  new_state.weight = 1.0f;
}
