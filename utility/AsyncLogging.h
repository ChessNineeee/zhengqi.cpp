#ifndef ZHENGQI_CPP_ASYNCLOGGING_H
#define ZHENGQI_CPP_ASYNCLOGGING_H

#include "BlockingQueue.h"
#include "BoundedBlockingQueue.h"
#include "CountDownLatch.h"
#include "LogStream.h"
#include "MutexLock.h"
#include "Thread.h"
#include "Types.h"
#include "noncopyable.h"

#include <atomic>
#include <memory>
#include <vector>

namespace zhengqi {
namespace utility {
class AsyncLogging : noncopyable {
public:
  AsyncLogging(const string &basename, off_t rollSize, int flushInterval = 3);

  ~AsyncLogging() {
    if (running_) {
      stop();
    }
  }
  void append(const char *logline, int len);

  void start() {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop() {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

private:
  void threadFunc();

  typedef FixedBuffer<kLargeBuffer> Buffer;
  typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
  typedef BufferVector::value_type BufferPtr;

  const int flushInterval_;
  std::atomic<bool> running_;
  const string basename_;
  const off_t rollSize_;
  Thread thread_;
  CountDownLatch latch_;
  MutexLock mutex_;
  Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};
} // namespace utility
} // namespace zhengqi

#endif // !ZHENGQI_CPP_ASYNCLOGGING_H
