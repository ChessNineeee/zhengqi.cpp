#include "networking/Timer.h"
#include "utility/Timestamp.h"

using namespace zhengqi::utility;
using namespace zhengqi::networking;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now) {
  if (repeat_) {
    expiration_ = addTime(now, interval_);
  } else {
    expiration_ = Timestamp::invalid();
  }
}
