//
// Created by 70903 on 2023/9/12.
//

#ifndef ZHENGQI_CPP_EPOLLPOLLER_H
#define ZHENGQI_CPP_EPOLLPOLLER_H

#include "networking/Poller.h"
#include <vector>

struct epoll_event;

namespace zhengqi {
namespace networking {
class EPollPoller : public Poller {
public:
  EPollPoller(EventLoop *loop);
  ~EPollPoller() override;

  utility::Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;

private:
  static const int kInitEventListSize = 16;

  static const char *operationToString(int op);

  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

  void update(int operation, Channel *channel);

  typedef std::vector<struct epoll_event> EventList;

  int epollfd_;
  EventList events_;
};
} // namespace networking
} // namespace zhengqi

#endif // ZHENGQI_CPP_EPOLLPOLLER_H
