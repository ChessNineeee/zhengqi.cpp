#ifndef ZHENGQI_CPP_NETWORKING_CHANNEL_H
#define ZHENGQI_CPP_NETWORKING_CHANNEL_H

#include "utility/Timestamp.h"
#include "utility/noncopyable.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <string>

namespace zhengqi {
namespace networking {

class EventLoop;

class Channel : utility::noncopyable {
public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void(utility::Timestamp)> ReadEventCallback;

  Channel(EventLoop *loop, int fd);
  ~Channel();

  void handleEvent(utility::Timestamp receiveTime);
  void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

  void tie(const std::shared_ptr<void> &);

  int fd() const { return fd_; }

  int events() const { return events_; }

  void set_revents(int revt) { revents_ = revt; }

  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading() {
    events_ |= kReadEvent;
    update();
  }

  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }

  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }

  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }

  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

  bool isWriting() const { return events_ & kWriteEvent; }
  bool isReading() const { return events_ & kReadEvent; }

  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  std::string reventsToString() const;
  std::string eventsToString() const;

  void doNotLogHup() { logHup_ = false; }

  EventLoop *ownerLoop() { return loop_; }
  void remove();

private:
  static std::string eventsToString(int fd, int ev);

  void update();
  void handleEventWithGuard(utility::Timestamp receiveTime);
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop *loop_;
  const int fd_;
  int events_;  /* events of interest on fd_ */
  int revents_; /* events that occured on fd_ */
  int index_;
  bool logHup_;

  std::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;
  bool addedToLoop_;

  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};
} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_CHANNEL_H
