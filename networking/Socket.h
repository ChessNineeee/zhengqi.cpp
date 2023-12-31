#ifndef ZHENGQI_CPP_NETWORKING_SOCKET_H
#define ZHENGQI_CPP_NETWORKING_SOCKET_H

#include "networking/InetAddress.h"
#include "utility/noncopyable.h"

struct tcp_info;

namespace zhengqi {
namespace networking {

class InetAddress;

class Socket : utility::noncopyable {
public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();

  int fd() const { return sockfd_; }

  bool getTcpInfo(struct tcp_info *) const;
  bool getTcpInfoString(char *buf, int len) const;

  void bindAddress(const InetAddress &localaddr);
  void listen();

  int accept(InetAddress *peeraddr);

  void shutdownWrite();

  void setTcpNoDelay(bool on);

  void setReuseAddr(bool on);

  void setReusePort(bool on);

  void setKeepAlive(bool on);

private:
  const int sockfd_;
};
} // namespace networking
} // namespace zhengqi
#endif // !ZHENGQI_CPP_NETWORKING_SOCKET_H
