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
