#include "animation.hpp"

Animation::Animation(std::string anim_name) : _name(anim_name) {}

Animation::Animation(Animation const& src) { *this = src; }

Animation::~Animation(void) {}

Animation& Animation::operator=(Animation const& rhs) {
  if (this != &rhs) {
  }
  return (*this);
}

void Animation::addChannel(std::string key, const AnimChannel& channel) {
  auto chan_it = channels.find(key);
  if (chan_it == channels.end()) {
    channels.emplace(key, channel);
  }
}
