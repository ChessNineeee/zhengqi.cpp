#ifndef ZHENGQI_CPP_NETWORKING_EVENTLOOPTHREADPOOL_H
#define ZHENGQI_CPP_NETWORKING_EVENTLOOPTHREADPOOL_H

#include "utility/Types.h"
#include "utility/noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

namespace zhengqi {
namespace networking {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : utility::noncopyable {

public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
  ~EventLoopThreadPool();

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback &cb = ThreadInitCallback());

  EventLoop *getNextLoop();

  EventLoop *getLoopForHash(size_t hashCode);

  std::vector<EventLoop *> getAllLoops();

  bool started() const { return started_; }

  const std::string &name() const { return name_; }

private:
  EventLoop *baseLoop_;
  std::string name_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *> loops_;
};

} // namespace networking
} // namespace zhengqi

#endif // !DEBUG
