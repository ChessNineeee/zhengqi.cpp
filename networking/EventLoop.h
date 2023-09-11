#ifndef ZHENGQI_CPP_NETWORKING_EVENTLOOP_H
#define ZHENGQI_CPP_NETWORKING_EVENTLOOP_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include <boost/any.hpp>

#include "networking/Callbacks.h"
#include "networking/TimerId.h"
#include "utility/CurrentThread.h"
#include "utility/MutexLock.h"
#include "utility/Timestamp.h"
#include "utility/noncopyable.h"

namespace zhengqi {
namespace networking {

class Channel;
class Poller;
class TimerQueue;

class EventLoop : utility::noncopyable {
public:
  typedef std::function<void()> Functor;
  EventLoop();
  ~EventLoop();

  void loop();

  void quit();

  utility::Timestamp pollReturnTime() const { return pollReturnTime_; }

  int64_t iteration() const { return iteration_; }

  void runInLoop(Functor cb);

  void queueInLoop(Functor cb);

  size_t queueSize() const;

  TimerId runAt(utility::Timestamp time, TimerCallback cb);
  TimerId runAfter(double delay, TimerCallback cb);
  TimerId runEvery(double interval, TimerCallback cb);

  void cancel(TimerId timerId);

  void wakeup();

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  bool isInLoopThread() const {
    return threadId_ == utility::CurrentThread::tid();
  }

  bool eventHandling() const { return eventHandling_; }

  void setContext(const boost::any &context) { context_ = context; }

  const boost::any &getContext() const { return context_; }

  boost::any *getMutableContext() { return &context_; }

  static EventLoop *getEventLoopOfCurrentThread();

private:
  void abortNotInLoopThread();
  void handleRead();
  void doPendingFunctors();

  void printActiveChannels() const;

  typedef std::vector<Channel *> ChannelList;

  bool looping_;
  std::atomic<bool> quit_;

  bool eventHandling_;
  bool callingPendingFunctors_;
  int64_t iteration_;
  const pid_t threadId_;
  utility::Timestamp pollReturnTime_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timerQueue_;
  int wakeupFd_;

  std::unique_ptr<Channel> wakeupChannel_;
  boost::any context_;

  ChannelList activeChannels_;
  Channel *currentActiveChannel;

  mutable utility::MutexLock mutex_;
  std::vector<Functor> pendingFunctors_;
};

} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_EVENTLOOP_H
