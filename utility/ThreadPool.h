//
// Created by zhengqi on 2023/9/3.
//

#ifndef CPP_THREADPOOL_H
#define CPP_THREADPOOL_H

#include "utility/Condition.h"
#include "utility/MutexLock.h"
#include "utility/Thread.h"
#include "utility/Types.h"
#include "utility/noncopyable.h"
#include <deque>
#include <functional>
#include <vector>

namespace zhengqi {
namespace utility {

class ThreadPool : noncopyable {
public:
  typedef std::function<void()> Task;

  explicit ThreadPool(const string &nameArg = string("ThreadPool"));
  ~ThreadPool();

  void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
  void setThreadInitCallback(const Task &cb) { threadInitCallback_ = cb; }

  void start(int numThreads);
  void stop();
  const string &name() const { return name_; }
  size_t queueSize() const;
  // Could block if maxQueueSize > 0
  // Call after stop() will return immediately.
  // There is no move-only version of std::function in C++ as of C++14.
  // So we don't need to overload a const& and an && versions
  // as we do in (Bounded)BlockingQueue.
  // https://stackoverflow.com/a/25408989
  void run(Task f);

private:
  bool isFull() const;
  void runInThread();
  Task take();

  mutable MutexLock mutex_;
  Condition notEmpty_; // GUARDED_BY(mutex_);
  Condition notFull_;  // GUARDED_BY(mutex_);
  string name_;
  Task threadInitCallback_;
  std::vector<std::unique_ptr<Thread>> threads_;
  std::deque<Task> queue_;
  size_t maxQueueSize_;
  bool running_;
};

} // namespace utility
} // namespace zhengqi

#endif // CPP_THREADPOOL_H
