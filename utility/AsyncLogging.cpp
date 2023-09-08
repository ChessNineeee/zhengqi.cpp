#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"
#include "utility/CountDownLatch.h"
#include "utility/MutexLock.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdio.h>

using namespace zhengqi::utility;

AsyncLogging::AsyncLogging(const string &basename, off_t rollSize,
                           int flushInterval)
    : flushInterval_(flushInterval), running_(false), basename_(basename),
      rollSize_(rollSize),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"), latch_(1),
      mutex_(), cond_(mutex_), currentBuffer_(new Buffer),
      nextBuffer_(new Buffer), buffers_() {
  currentBuffer_->bZero();
  nextBuffer_->bZero();
  buffers_.reserve(16);
}

void AsyncLogging::append(const char *logline, int len) {
  MutexLockGuard lock(mutex_);
  if (currentBuffer_->avail() > len) {
    currentBuffer_->append(logline, static_cast<size_t>(len));
  } else {
    buffers_.push_back(std::move(currentBuffer_));
    if (nextBuffer_) {
      currentBuffer_ = std::move(nextBuffer_);
    } else {
      currentBuffer_.reset(new Buffer);
    }
    currentBuffer_->append(logline, static_cast<size_t>(len));
    cond_.notify();
  }
}

void AsyncLogging::threadFunc() {
  assert(running_ == true);
  latch_.countDown();
  // 日志输出文件 output
  LogFile output(basename_, rollSize_, false);
  // 4MB 临时缓冲区
  BufferPtr newBuffer1(new Buffer);
  // 4MB 临时缓冲区
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bZero();
  newBuffer2->bZero();
  BufferVector buffersToWrite;
  // 将缓冲区数组的容量设置为16 --> 初始时buffersToWrite有16个缓冲区
  buffersToWrite.reserve(16);
  while (running_) {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      MutexLockGuard lock(mutex_);
      if (buffers_.empty()) {
        cond_.waitForSeconds(flushInterval_);
      }
      buffers_.push_back(std::move(currentBuffer_));
      currentBuffer_ = std::move(newBuffer1);
      // 将 buffersToWrite 与 buffers_
      // swap，线程处理buffersToWrite中待写入的缓冲区
      buffersToWrite.swap(buffers_);
      if (!nextBuffer_) {
        nextBuffer_ = std::move(newBuffer2);
      }
    }

    assert(!buffersToWrite.empty());
    if (buffersToWrite.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof buf,
               "Dropped log message at %s, %zd larger buffers\n",
               Timestamp::now().toFormattedString().c_str(),
               buffersToWrite.size() - 2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      // 只留头两个缓冲区
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }

    for (const auto &buffer : buffersToWrite) {
      output.append(buffer->data(), buffer->length());
    }

    if (buffersToWrite.size() > 2) {
      buffersToWrite.resize(2);
    }

    if (!newBuffer1) {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2) {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2.reset();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}
