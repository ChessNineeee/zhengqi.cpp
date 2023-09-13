//
// Created by 70903 on 2023/9/13.
//

#include "networking/Channel.h"
#include "networking/EventLoop.h"
#include "utility/Logging.h"

#include <map>
#include <sys/timerfd.h>

using namespace zhengqi::networking;
using namespace zhengqi::utility;

void print(const char *msg) {
  static std::map<const char *, Timestamp> lasts;
  Timestamp &last = lasts[msg];
  Timestamp now = Timestamp::now();
  printf("%s tid %d %s delay %f\n", now.toString().c_str(),
         CurrentThread::tid(), msg, timeDifference(now, last));
  last = now;
}

namespace zhengqi {
namespace networking {
namespace detail {
int createTimerfd();
void readTimerfd(int fd, Timestamp receiveTime);
} // namespace detail
} // namespace networking
} // namespace zhengqi

class PeriodicTimer {
public:
  PeriodicTimer(EventLoop *loop, double interval, const TimerCallback &cb)
      : loop_(loop), timerfd_(detail::createTimerfd()),
        timerfdChannel_(loop, timerfd_), interval_(interval), cb_(cb) {
    timerfdChannel_.setReadCallback(
        std::bind(&PeriodicTimer::handleRead, this));
    timerfdChannel_.enableReading();
  }

  void start() {
    struct itimerspec spec;
    memZero(&spec, sizeof spec);
    spec.it_interval = toTimeSpec(interval_);
    spec.it_value = spec.it_interval;
    int ret = ::timerfd_settime(timerfd_, 0, &spec, nullptr);
    if (ret) {
      LOG_SYSERR << "timerfd_settime()";
    }
  }

  ~PeriodicTimer() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
  }

private:
  void handleRead() {
    loop_->assertInLoopThread();
    detail::readTimerfd(timerfd_, Timestamp::now());
    if (cb_) {
      cb_();
    }
  }

  static struct timespec toTimeSpec(double seconds) {
    struct timespec ts;
    memZero(&ts, sizeof ts);
    const int64_t kNanoSecondsPerSecond = 1000000000;
    const int kMinInterval = 100000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
    if (nanoseconds < kMinInterval) {
      nanoseconds = kMinInterval;
    }
    ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
    return ts;
  }

  EventLoop *loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  const double interval_;
  TimerCallback cb_;
};

int main() {
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid()
           << " Try adjusting the wall clock, see what happens.";
  EventLoop loop;
  PeriodicTimer timer(&loop, 1, std::bind(print, "PeriodicTimer"));
  timer.start();
  loop.runEvery(1, std::bind(print, "EventLoop::runEvery"));
  loop.loop();
}