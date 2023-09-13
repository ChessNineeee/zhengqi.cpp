#ifndef ZHENGQI_CPP_NETWORKING_EVENTLOOPTHREAD_H
#define ZHENGQI_CPP_NETWORKING_EVENTLOOPTHREAD_H

#include "utility/Condition.h"
#include "utility/MutexLock.h"
#include "utility/Thread.h"
#include "utility/noncopyable.h"
#include <functional>

namespace zhengqi {
namespace networking {
class EventLoop;
class EventLoopThread : utility::noncopyable {

public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                  const std::string &name = std::string());
  ~EventLoopThread();
  EventLoop *startLoop();

private:
  void threadFunc();
  EventLoop *loop_;
  bool exiting_;
  utility::Thread thread_;
  utility::MutexLock mutex_;
  utility::Condition cond_;
  ThreadInitCallback callback_;
};

} // namespace networking
} // namespace zhengqi
#endif // !ZHENGQI_CPP_NETWORKING_EVENTLOOPTHREAD_H
