#ifndef ZHENGQI_CPP_ASYNCLOGGING_H
#define ZHENGQI_CPP_ASYNCLOGGING_H

#include "utility/BlockingQueue.h"
#include "utility/BoundedBlockingQueue.h"
#include "utility/CountDownLatch.h"
#include "utility/LogStream.h"
#include "utility/MutexLock.h"
#include "utility/Thread.h"
#include "utility/Types.h"
#include "utility/noncopyable.h"

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

  const int flushInterval_; // 刷盘周期
  std::atomic<bool> running_;
  const string basename_;
  const off_t rollSize_;
  Thread thread_;
  CountDownLatch latch_;
  MutexLock mutex_;
  Condition cond_;
  BufferPtr currentBuffer_; // 当前缓冲区，负责接收用户产生的日志信息
  BufferPtr nextBuffer_; // 预备缓冲区
  BufferVector buffers_; // 存放供后端写入的所有缓冲区
};
} // namespace utility
} // namespace zhengqi

#endif // !ZHENGQI_CPP_ASYNCLOGGING_H
