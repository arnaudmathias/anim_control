#include "animation.hpp"

Animation::Animation(std::string anim_name, double duration,
                     double ticks_per_second)
    : _name(anim_name),
      _duration(duration),
      _ticks_per_second(ticks_per_second) {}

Animation::Animation(Animation const& src) { *this = src; }

Animation::~Animation(void) {}

Animation& Animation::operator=(Animation const& rhs) {
  if (this != &rhs) {
    channels = rhs.channels;
    _name = rhs._name;
    _duration = rhs._duration;
    _ticks_per_second = rhs._ticks_per_second;
  }
  return (*this);
}

void Animation::addChannel(std::string key, const AnimChannel& channel) {
  auto chan_it = channels.find(key);
  if (chan_it == channels.end()) {
    channels.emplace(key, channel);
  }
}
