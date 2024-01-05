#include "networking/InetAddress.h"
#include "networking/Socket.h"
#include "networking/SocketsOps.h"
#include "networking/TcpConnection.h"
#include "utility/Assert2.h"
#include "utility/Logging.h"
#include <co/co.h>
#include <co/co/sock.h>
#include <fcntl.h>
#include <sys/socket.h>

#define DOMAIN_SIZE_LIMIT 512
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define IP_ADDR_SIZE 4
#define BUFSIZE 4096
#define NOAUTH 0x00
#define SOCKS5_VERSION 0x05
#define SOCKS5_RSV 0x00
#define SOCKS5_CONNECT 0x01
#define SOCKS5_CONNECT_HEADER_SIZE 0x04
#define SOCKS5_HOST_IN_IPv4 0x01
#define SOCKS5_HOST_IN_DOMAIN 0x03
#define SOCKS5_CONNECT_OK 0x00
#define HANDSHAKE 0x01
#define CONNECT 0x02
#define CONNECT2 0x03
#define CONNECTING 0x04

using namespace zhengqi::utility;
using namespace zhengqi::networking;

struct conn {
  int fd;
  sockaddr_in addr;
};

ssize_t /* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, ssize_t n) {
  ssize_t nleft;
  ssize_t nread;
  char *ptr;

  ptr = static_cast<char *>(vptr);
  nleft = n;
  while (nleft > 0) {
    if ((nread = co::recv(fd, ptr, static_cast<int>(nleft))) < 0) {
      if (errno == EINTR)
        nread = 0; /* and call read() again */
      else {
        LOG_ERROR << errno;
        return (-1);
      }
    } else if (nread == 0)
      break; /* EOF */

    nleft -= nread;
    ptr += nread;
  }
  return (n - nleft); /* return >= 0 */
}

ssize_t /* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, ssize_t n) {
  ssize_t nleft;
  ssize_t nwritten;
  const char *ptr;

  ptr = static_cast<const char *>(vptr);
  nleft = n;
  while (nleft > 0) {
    if ((nwritten = co::send(fd, ptr, static_cast<int>(nleft))) <= 0) {
      if (nwritten < 0 && errno == EINTR)
        nwritten = 0; /* and call write() again */
      else
        return (-1); /* error */
    }

    nleft -= nwritten;
    ptr += nwritten;
  }
  return (n);
}

static void *handle_socks5_client(void *p) {
  struct conn *cli_conn = static_cast<struct conn *>(p);
  assert_2(cli_conn != nullptr, "parameter should never be nil");
  std::unique_ptr<struct conn> cli_conn_ptr(cli_conn);

  int flags = fcntl(cli_conn_ptr->fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  flags |= O_NDELAY;
  ssize_t ret = fcntl(cli_conn_ptr->fd, F_SETFL, flags);
  assert(0 == ret);

  InetAddress cli_addr(cli_conn_ptr->addr);

  LOG_DEBUG << "handle client: " << cli_addr.toIpPort();

  char init[3];
  ssize_t nread =
      co::recvn(cli_conn_ptr->fd, static_cast<void *>(init), ARRAY_SIZE(init));
  assert_2(nread == ARRAY_SIZE(init), "nread should never fail" << nread);
  assert_2(SOCKS5_VERSION == init[0], "wrong byte");
  assert_2(0x01 == init[1], "wrong byte");
  assert_2(NOAUTH == init[2], "wrong byte");

  char reply[2] = {SOCKS5_VERSION, NOAUTH};
  ret =
      co::send(cli_conn_ptr->fd, static_cast<void *>(reply), ARRAY_SIZE(reply));
  assert_2(ARRAY_SIZE(reply) == ret, "nwrite should never fail");
  LOG_DEBUG << "handshake finished: " << cli_addr.toIpPort();

  char connect_header[4];
  nread = co::recvn(cli_conn_ptr->fd, static_cast<void *>(connect_header),
                    ARRAY_SIZE(connect_header));

  assert_2(ARRAY_SIZE(connect_header) == nread, "nread should never fail");
  assert_2(connect_header[0] == SOCKS5_VERSION, "wrong byte");
  assert_2(connect_header[1] == SOCKS5_CONNECT, "wrong byte");
  assert_2(connect_header[2] == SOCKS5_RSV, "wrong byte");
  assert_2(connect_header[3] == SOCKS5_HOST_IN_IPv4 ||
               connect_header[3] == SOCKS5_HOST_IN_DOMAIN,
           "wrong byte");
  char remote_addr_type = connect_header[3];
  sockaddr_in remote_addr;
  memZero(&remote_addr, sizeof remote_addr);
  remote_addr.sin_family = AF_INET;

  char ip_port[6];
  unsigned char domain_len;
  char domain[DOMAIN_SIZE_LIMIT];
  char port[2];
  if (remote_addr_type == SOCKS5_HOST_IN_IPv4) {
    ret = co::recvn(cli_conn_ptr->fd, static_cast<void *>(ip_port),
                    ARRAY_SIZE(ip_port));

    assert_2(ARRAY_SIZE(ip_port) == ret, "nread should never fail");

    const void *remote_ip = ip_port;
    const void *remote_port = ip_port + IP_ADDR_SIZE;

    remote_addr.sin_port = *static_cast<const in_port_t *>(remote_port);
    remote_addr.sin_addr.s_addr = *static_cast<const uint32_t *>(remote_ip);
  } else {
    ret = co::recvn(cli_conn_ptr->fd, static_cast<void *>(&domain_len),
                    sizeof(domain_len));
    assert_2(sizeof(domain_len) == ret, "nread should never fail");
    ret = co::recvn(cli_conn_ptr->fd, static_cast<void *>(domain),
                    static_cast<int>(domain_len));

    assert_2(domain_len == ret, "readn should never fail");
    domain[domain_len] = 0;

    InetAddress tmp;
    bool resolv_ret = InetAddress::resolve(domain, &tmp);
    if (resolv_ret) {
      remote_addr.sin_addr.s_addr = tmp.ipv4NetEndian();
      ret = co::recvn(cli_conn_ptr->fd, static_cast<void *>(port),
                      ARRAY_SIZE(port));
      assert_2(ARRAY_SIZE(port) == ret, "readn should never fail");
      const void *remote_port = port;
      remote_addr.sin_port = *static_cast<const in_port_t *>(remote_port);
    } else {
      assert_2(false, "resolve host should never fail");
    }
  }
  int remote_fd = co::tcp_socket(remote_addr.sin_family);
  void *remote_addr_ptr = &remote_addr;
  ret = co::connect(remote_fd, static_cast<struct sockaddr *>(remote_addr_ptr),
                    sizeof(remote_addr));
  assert_2(0 == ret, "connect should never fail");

  char connect_reply[4] = {SOCKS5_VERSION, SOCKS5_CONNECT_OK, SOCKS5_RSV,
                           remote_addr_type};
  ret = co::send(cli_conn_ptr->fd, static_cast<void *>(connect_reply),
                 ARRAY_SIZE(connect_reply));
  assert_2(ARRAY_SIZE(connect_reply) == ret, "writen should never fail");

  if (SOCKS5_HOST_IN_IPv4 == remote_addr_type) {
    ret = co::send(cli_conn_ptr->fd, static_cast<void *>(ip_port),
                   ARRAY_SIZE(ip_port));
    assert_2(ARRAY_SIZE(ip_port) == ret, "write n should never fail");
  } else {
    ret = co::send(cli_conn_ptr->fd, static_cast<void *>(&domain_len),
                   sizeof(domain_len));

    assert_2(static_cast<ssize_t>(sizeof(domain_len)) == ret,
             "writen should never fail");
    ret = co::send(cli_conn_ptr->fd, static_cast<void *>(domain),
                   static_cast<int>(strlen(domain)));
    assert_2(static_cast<ssize_t>(strlen(domain)) == ret,
             "writen should never fail");
    ret =
        co::send(cli_conn_ptr->fd, static_cast<void *>(port), ARRAY_SIZE(port));
    assert_2(ARRAY_SIZE(port) == ret, "writen should never fail");
  }

  LOG_DEBUG << "connect remote finished: " << cli_addr.toIpPort();

  while (1) {
    int byte_rcv, byte_snd;
    char buffer[BUFSIZE];
    byte_rcv = co::recv(cli_conn_ptr->fd, static_cast<void *>(buffer), BUFSIZE);
    LOG_DEBUG << "read: " << byte_rcv << "bytes from client";
    if (byte_rcv <= 0)
      break;
    byte_snd = co::send(remote_fd, static_cast<void *>(buffer),
                        static_cast<int>(byte_rcv));
    LOG_DEBUG << "send: " << byte_snd << "bytes to server";
    assert_2(byte_snd > 0, "write should never fail");

    byte_rcv = co::recv(remote_fd, static_cast<void *>(buffer), BUFSIZE);
    LOG_DEBUG << "read: " << byte_rcv << "bytes from server";
    if (byte_rcv <= 0)
      break;
    byte_snd = co::send(cli_conn_ptr->fd, static_cast<void *>(buffer),
                        static_cast<int>(byte_rcv));
    LOG_DEBUG << "send: " << byte_snd << "bytes to client";
    assert_2(byte_snd > 0, "write should never fail");
  }

  LOG_DEBUG << "connection finished: " << cli_addr.toIpPort();
  close(remote_fd);
  close(cli_conn_ptr->fd);
  return NULL;
}

static void *accept_routine(void *) {
  InetAddress listenAddr(1080);
  int listener_fd = sockets::createNonblockingOrDie(listenAddr.family());
  assert_2(listener_fd >= 0, "fd should always larger than 1");
  Socket listener(listener_fd);
  listener.bindAddress(listenAddr);
  listener.setReuseAddr(true);
  listener.setTcpNoDelay(true);
  listener.listen();

  LOG_DEBUG << "listening on " << listenAddr.toIpPort();
  for (;;) {
    struct conn *cli_conn = new struct conn();
    int len = sizeof(cli_conn->addr);
    void *cli_addr_ptr = &(cli_conn->addr);
    int client_fd = co::accept(
        listener.fd(), static_cast<struct sockaddr *>(cli_addr_ptr), &len);

    assert_2(client_fd >= 0,
             "fd should always larger than 1: " << client_fd << errno << len);

    cli_conn->fd = client_fd;

    go([](struct conn *conn) { handle_socks5_client(conn); }, cli_conn);
    cli_conn = nullptr;
  }
}

int main() {
  auto s = co::main_sched();
  go(accept_routine, static_cast<void *>(NULL));
  s->loop();
  return 0;
}