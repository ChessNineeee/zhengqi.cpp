#ifndef ZHENGQI_CPP_NETWORKING_ACCEPTOR_H
#define ZHENGQI_CPP_NETWORKING_ACCEPTOR_H

#include "networking/Channel.h"
#include "networking/Socket.h"
#include "utility/noncopyable.h"
#include <functional>

namespace zhengqi {
namespace networking {

class EventLoop;
class InetAddress;

class Acceptor : utility::noncopyable {
public:
  typedef std::function<void(int sockfd, const InetAddress &)>
      NewConnectionCallback;

  Acceptor(EventLoop *loop, const InetAddress &, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback &cb) {
    newConnectionCallback_ = cb;
  }

  void listen();

  bool listening() const { return listening_; }

private:
  void handleRead();

  EventLoop *loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listening_;
  int idleFd_;
};
} // namespace networking
} // namespace zhengqi
#endif
