#ifndef ZHENGQI_CPP_NETWORKING_TIMER_H
#define ZHENGQI_CPP_NETWORKING_TIMER_H

#include "networking/Callbacks.h"
#include "utility/Atomic.h"
#include "utility/Timestamp.h"
#include "utility/noncopyable.h"
#include <cstdint>

namespace zhengqi {
namespace networking {

class Timer : utility::noncopyable {

public:
  Timer(TimerCallback cb, utility::Timestamp when, double interval)
      : callback_(std::move(cb)), expiration_(when), interval_(interval),
        repeat_(interval > 0.0), sequence_(s_numCreated_.incrementAndGet()) {}

  void run() const { callback_(); }

  utility::Timestamp expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }

  int64_t sequence() const { return sequence_; }

  void restart(utility::Timestamp now);

  static int64_t numCreated() { return s_numCreated_.get(); }

private:
  const TimerCallback callback_;
  utility::Timestamp expiration_;
  const double interval_;
  const bool repeat_;
  const int64_t sequence_;

  static utility::AtomicInt64 s_numCreated_;
};
} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_TIMER_H
