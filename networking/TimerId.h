#ifndef ZHENGQI_CPP_NETWORKING_TIMERID_H
#define ZHENGQI_CPP_NETWORKING_TIMERID_H

#include "utility/copyable.h"

#include <stdint.h>

namespace zhengqi {
namespace networking {
class Timer;

class TimerId : public utility::copyable {
public:
  TimerId() : timer_(nullptr), sequence_(0) {}
  TimerId(Timer *timer, int64_t seq) : timer_(timer), sequence_(seq) {}

  friend class TimerQueue;

private:
  Timer *timer_;
  int64_t sequence_;
};
} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_TIMERID_H
