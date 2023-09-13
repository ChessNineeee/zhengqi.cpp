#include "networking/EventLoopThread.h"
#include "networking/EventLoop.h"
#include "utility/MutexLock.h"
#include <cassert>

using namespace zhengqi::networking;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const std::string &name)
    : loop_(NULL), exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this), name), mutex_(),
      cond_(mutex_), callback_(cb) {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != NULL) {
    loop_->quit();
    thread_.join();
  }
}

EventLoop *EventLoopThread::startLoop() {
  assert(!thread_.started());
  thread_.start();

  EventLoop *loop = NULL;
  {
    utility::MutexLockGuard lock(mutex_);
    while (loop_ == NULL) {
      cond_.wait();
    }
    loop = loop_;
  }
  return loop;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;

  if (callback_) {
    callback_(&loop);
  }

  {
    utility::MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop.loop();

  // assert(exiting_);
  utility::MutexLockGuard lock(mutex_);
  loop_ = NULL;
}
