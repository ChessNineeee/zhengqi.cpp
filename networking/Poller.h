#ifndef ZHENGQI_CPP_NETWORKING_POLLER_H
#define ZHENGQI_CPP_NETWORKING_POLLER_H

#include <map>
#include <vector>

#include "networking/EventLoop.h"
#include "utility/Timestamp.h"
#include "utility/noncopyable.h"

namespace zhengqi {
namespace networking {

class Channel;

class Poller : utility::noncopyable {

public:
  typedef std::vector<Channel *> ChannelList;
  Poller(EventLoop *loop);
  virtual ~Poller();

  virtual utility::Timestamp poll(int timeoutMs,
                                  ChannelList *activeChannels) = 0;

  virtual void updateChannel(Channel *channel) = 0;

  virtual void removeChannel(Channel *channel) = 0;

  virtual bool hasChannel(Channel *channel) const;

  static Poller *newDefaultPoller(EventLoop *loop);

  void assertInLoopThread() const { ownerLoop_->assertInLoopThread(); }

protected:
  typedef std::map<int, Channel *> ChannelMap;
  ChannelMap channels_;

private:
  EventLoop *ownerLoop_;
};

} // namespace networking
} // namespace zhengqi
#endif // !DEBUG
