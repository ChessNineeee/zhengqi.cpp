#ifndef ZHENGQI_CPP_NETWORKING_TIMERQUEUE_H
#define ZHENGQI_CPP_NETWORKING_TIMERQUEUE_H

#include <set>
#include <vector>

#include "networking/Callbacks.h"
#include "networking/Channel.h"
#include "utility/MutexLock.h"
#include "utility/Timestamp.h"

namespace zhengqi {
namespace networking {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : utility::noncopyable {
public:
  explicit TimerQueue(EventLoop *loop);
  ~TimerQueue();

  TimerId addTimer(TimerCallback cb, utility::Timestamp when, double interval);

  void cancel(TimerId timerId);

private:
  typedef std::pair<utility::Timestamp, Timer *> Entry;
  typedef std::set<Entry> TimerList;
  typedef std::pair<Timer *, int64_t> ActiveTimer;
  typedef std::set<ActiveTimer> ActiveTimerSet;

  void addTimerInLoop(Timer *timer);
  void cancelInLoop(TimerId timerId);

  void handleRead();

  std::vector<Entry> getExpired(utility::Timestamp now);
  void reset(const std::vector<Entry> &expired, utility::Timestamp now);

  bool insert(Timer *timer);

  EventLoop *loop_;
  const int timerfd_;
  Channel timerfdChannel_;

  ActiveTimerSet activeTimers_;
  bool callingExpiredTimers_;
  ActiveTimerSet cancelingTimers_;
};
} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_TIMERQUEUE_H
