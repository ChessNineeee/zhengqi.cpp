//
// Created by 70903 on 2023/9/12.
//

#include "networking/Poller.h"
#include "networking/poller/EPollPoller.h"
#include "networking/poller/PollPoller.h"

using namespace zhengqi::networking;

Poller *Poller::newDefaultPoller(EventLoop *loop) {
  if (::getenv("ZHENGQI_USE_POLL")) {
    return new PollPoller(loop);
  } else {
    return new EPollPoller(loop);
  }
}
