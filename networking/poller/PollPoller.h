//
// Created by 70903 on 2023/9/12.
//

#ifndef ZHENGQI_CPP_POLLPOLLER_H
#define ZHENGQI_CPP_POLLPOLLER_H

#include "networking/Poller.h"
#include <vector>

struct pollfd;
namespace zhengqi {
namespace networking {

class PollPoller : public Poller {
public:
  PollPoller(EventLoop *loop);
  ~PollPoller() override;

  utility::Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;

private:
  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

  typedef std::vector<struct pollfd> PollFdList;

  PollFdList pollfds_;
};
} // namespace networking
} // namespace zhengqi

#endif // ZHENGQI_CPP_POLLPOLLER_H
