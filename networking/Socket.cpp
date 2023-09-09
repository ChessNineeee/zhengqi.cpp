#include "networking/Socket.h"
#include "utility/Logging.h"
#include "utility/Types.h"

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace zhengqi::utility;
using namespace zhengqi::networking;

Socket::~Socket() {}

bool Socket::getTcpInfo(struct tcp_info *tcpi) const {
  socklen_t len = sizeof(*tcpi);
  memZero(tcpi, len);
  return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len);
}

bool Socket::getTcpInfoString(char *buf, int len) const {
  struct tcp_info tcpi;
  bool ok = getTcpInfo(&tcpi);
  if (ok) {
    snprintf(buf, static_cast<size_t>(len),
             "unrecovered=%u "
             "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
             "lost=%u retrans=%u rtt=%u rttvar=%u "
             "sshthresh=%u cwnd=%u total_retrans=%u",
             tcpi.tcpi_retransmits, tcpi.tcpi_rto, tcpi.tcpi_ato,
             tcpi.tcpi_snd_mss, tcpi.tcpi_rcv_mss, tcpi.tcpi_lost,
             tcpi.tcpi_retrans, tcpi.tcpi_rtt, tcpi.tcpi_rttvar,
             tcpi.tcpi_snd_ssthresh, tcpi.tcpi_snd_cwnd,
             tcpi.tcpi_total_retrans);
  }
  return ok;
}

void Socket::bindAddress(const InetAddress &addr) {}

void Socket::listen() {}

int Socket::accept(const InetAddress &addr) {}

void Socket::shutdownWrite() {}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
               static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                         static_cast<socklen_t>(sizeof optval));

  if (ret < 0 && on) {
    LOG_SYSERR << "SO_REUSEPORT failed.";
  }
#else
  if (on) {
    LOG_ERROR << "SO_REUSEPORT is not supported.";
  }
#endif // DEBUG
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
               static_cast<socklen_t>(sizeof optval));
}
