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

/// 前端生产者核心实现
void AsyncLogging::append(const char *logline, int len) {
    MutexLockGuard lock(mutex_);
    if (currentBuffer_->avail() > len) {
        // 最常见的情况，当前缓冲区没满，日志信息直接拷贝(memcpy)进缓冲区
        currentBuffer_->append(logline, static_cast<size_t>(len));
    } else {
        // 当前缓冲区已满，将其push_back到buffers_中等待后端刷盘，这里使用std::move()将Buffer的所有权交给容器中的unique_ptr
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_) {
            // 有已经预先分配好的预备缓冲区，则直接作为当前缓冲区
            // std::move(unique_ptr<Buffer>) --> 移交Buffer所有权
            currentBuffer_ = std::move(nextBuffer_);
        } else {
            // 没有预备缓冲区，则重新分配一个新的Buffer作为当前缓冲区
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, static_cast<size_t>(len));
        // 此时 buffers_中一定至少有一个缓冲区待处理，因此唤醒后端线程
        cond_.notify();
    }
}

void AsyncLogging::threadFunc() {
    assert(running_ == true);
    latch_.countDown();
    // 日志输出文件 output
    LogFile output(basename_, rollSize_, false);
    // BufferPtr 是一个unique_ptr 对象
    BufferPtr newBuffer1(new Buffer);
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
        // 后端临界区
        {
            MutexLockGuard lock(mutex_);
            if (buffers_.empty()) {
                cond_.waitForSeconds(flushInterval_);
            }
            // 要么刷盘周期到了，要么buffers_不为空
            // 将当前缓冲区 push_back 到 buffers_中
            // 并将newBuffers1所持有的Buffer交给当前缓冲区
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            // 将buffers_中待处理的缓冲区全部移交给buffersToWrite
            // 此后后端可以在临界区外安全的处理buffersToWrite中的缓冲区，将数据写入文件
            buffersToWrite.swap(buffers_);
            // 如果预备缓冲区不持有Buffer，则将newBuffer2的缓冲区交给预备缓冲区
            // 这样前端总有一个额外的预备缓冲区可用，降低额外分配缓冲区的概率
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

        // 将缓冲区中的数据刷盘
        for (const auto &buffer: buffersToWrite) {
            output.append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2) {
            buffersToWrite.resize(2);
        }

        // 重新填充newBuffer1
        if (!newBuffer1) {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        // 重新填充newBuffer2
        if (!newBuffer2) {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        // 这样循环下次执行时，仍然有两个空闲的缓冲区替换当前缓冲区和预备缓冲区

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}
