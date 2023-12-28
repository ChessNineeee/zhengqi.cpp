#include "networking-examples/socks4a/tunnel.h"
#include "networking/Callbacks.h"
#include "networking/Endian.h"
#include "networking/EventLoop.h"
#include "networking/TcpServer.h"
#include "utility/CurrentThread.h"
#include "utility/ThreadLocal.h"
#include "utility/Types.h"
#include <algorithm>
#include <boost/any.hpp>
#include <fmt/core.h>
#include <iostream>
#include <map>
#include <netdb.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <unordered_set>

#define DOMAIN_SIZE_LIMIT 512
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

#define assert_2(condition, message)                                           \
  (!(condition))                                                               \
      ? (std::cerr << "Assertion failed: (" << #condition << "), "             \
                   << "function " << __FUNCTION__ << ", file " << __FILE__     \
                   << ", line " << __LINE__ << "." << std::endl                \
                   << message << std::endl,                                    \
         abort(), 0)                                                           \
      : 1

using namespace zhengqi::utility;
using namespace zhengqi::networking;

EventLoop *g_eventLoop;
ThreadLocal<std::map<string, TunnelPtr>> t_tunnels;

void onServerConnection(const TcpConnectionPtr &conn) {
  assert_2(conn != nullptr, "parameter should never be nil");
  LOG_DEBUG << conn->peerAddress().toIpPort()
            << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected()) {
    conn->setTcpNoDelay(true);
    assert_2(conn->getContext().empty(),
             "conn to be socks conn should never has context");
    conn->setContext(HANDSHAKE);
  } else {
    auto &tunnels = t_tunnels.value();
    std::map<string, TunnelPtr>::iterator it =
        tunnels.find(conn->peerAddress().toIpPort());
    if (it != tunnels.end()) {
      it->second->disconnect();
      tunnels.erase(it);
      LOG_DEBUG << "tunnel object erased, conn: "
                << conn->peerAddress().toIpPort() << " map: " << &tunnels;
    }
  }
}

void onServerMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp) {
  assert_2(conn != nullptr && buf != nullptr, "parameters should never be nil");
  LOG_DEBUG << conn->peerAddress().toIpPort() << " " << buf->readableBytes();

  auto context = conn->getContext();
  assert_2(!context.empty(), "socks conn should never has empty context");

  if (context.type() == typeid(int)) {
    int flag = boost::any_cast<int>(context);

    LOG_DEBUG << conn->peerAddress().toIpPort() << " " << flag;
    if (flag == HANDSHAKE) {
      assert_2(buf->readableBytes() <= 3, "too large message");
      if (buf->readableBytes() < 3)
        return;

      LOG_DEBUG << conn->peerAddress().toIpPort() << " first handshake";
      assert_2(buf->peek()[0] == SOCKS5_VERSION,
               "malformed byte" << buf->peek()[0]);
      assert_2(buf->peek()[1] == 0x01, "malformed byte" << buf->peek()[1]);
      assert_2(buf->peek()[2] == NOAUTH, "malformed byte" << buf->peek()[2]);
      buf->retrieve(3);

      char reply[2] = {SOCKS5_VERSION, NOAUTH};
      conn->send(reply, 2);

      conn->setContext(CONNECT);
    } else if (flag == CONNECT) {
      assert_2(buf->readableBytes() < DOMAIN_SIZE_LIMIT, "too large message");

      LOG_DEBUG << conn->peerAddress().toIpPort() << " connect remote";
      assert_2(buf->peek()[0] == SOCKS5_VERSION,
               "malformed byte" << buf->peek()[0]);
      assert_2(buf->peek()[1] == SOCKS5_CONNECT,
               "malformed byte" << buf->peek()[1]);
      assert_2(buf->peek()[2] == SOCKS5_RSV,
               "malformed byte" << buf->peek()[2]);

      char host_name_type = buf->peek()[3];
      assert_2(host_name_type == SOCKS5_HOST_IN_IPv4 ||
                   host_name_type == SOCKS5_HOST_IN_DOMAIN,
               "malformed byte: " << host_name_type);

      sockaddr_in remote_addr;
      memZero(&remote_addr, sizeof remote_addr);
      remote_addr.sin_family = AF_INET;

      size_t processed_bytes = 0;
      if (host_name_type == SOCKS5_HOST_IN_IPv4) {
        size_t connect_bytes_len =
            SOCKS5_CONNECT_HEADER_SIZE + IP_ADDR_SIZE + 2;
        assert_2(
            buf->readableBytes() <= connect_bytes_len,
            "readable bytes num should never larger than connect message size");
        if (buf->readableBytes() < connect_bytes_len)
          return;
        processed_bytes = IP_ADDR_SIZE + 2;
        const void *remote_ip = buf->peek() + SOCKS5_CONNECT_HEADER_SIZE;
        const void *remote_port =
            buf->peek() + IP_ADDR_SIZE + SOCKS5_CONNECT_HEADER_SIZE;
        remote_addr.sin_port = *static_cast<const in_port_t *>(remote_port);
        remote_addr.sin_addr.s_addr = *static_cast<const uint32_t *>(remote_ip);
      } else {
        const void *host_name_len_ptr =
            buf->peek() + SOCKS5_CONNECT_HEADER_SIZE;
        const int8_t host_name_len =
            *static_cast<const int8_t *>(host_name_len_ptr);
        size_t connect_bytes_len =
            SOCKS5_CONNECT_HEADER_SIZE + host_name_len + 2 + 1;

        assert_2(
            buf->readableBytes() <= connect_bytes_len,
            "readable bytes num should never larger than connect message size");
        if (buf->readableBytes() < connect_bytes_len)
          return;

        processed_bytes = host_name_len + 2 + 1;
        const char *remote_host_name =
            buf->peek() + SOCKS5_CONNECT_HEADER_SIZE + 1;
        const void *remote_port =
            buf->peek() + SOCKS5_CONNECT_HEADER_SIZE + 1 + host_name_len;

        char host_name[DOMAIN_SIZE_LIMIT];
        memZero(host_name, DOMAIN_SIZE_LIMIT);
        memcpy(host_name, remote_host_name, host_name_len);
        InetAddress tmp;
        bool resolv_ret = InetAddress::resolve(host_name, &tmp);
        if (resolv_ret) {
          remote_addr.sin_addr.s_addr = tmp.ipv4NetEndian();
          remote_addr.sin_port = *static_cast<const in_port_t *>(remote_port);
        } else {
          assert_2(false, "resolve host should never fail");
        }
      }

      InetAddress remote_iaddr(remote_addr);

      TunnelPtr tunnel(new Tunnel(g_eventLoop, remote_iaddr, conn));
      tunnel->setup();
      auto &tunnels = t_tunnels.value();
      tunnels[conn->peerAddress().toIpPort()] = tunnel;
      LOG_DEBUG << "connect to Remote: " << remote_iaddr.toIpPort()
                << " conn: " << conn->peerAddress().toIpPort()
                << "map: " << &tunnels << " map size: " << tunnels.size();

      char reply[4] = {SOCKS5_VERSION, SOCKS5_CONNECT_OK, SOCKS5_RSV,
                       host_name_type};
      conn->send(reply, 4);
      conn->send(buf->peek() + SOCKS5_CONNECT_HEADER_SIZE,
                 static_cast<int>(processed_bytes));
      conn->setContext(CONNECT2);

      assert_2(buf->readableBytes() ==
                   SOCKS5_CONNECT_HEADER_SIZE + processed_bytes,
               "buf size: " << buf->readableBytes() << " fixed size: "
                            << SOCKS5_CONNECT_HEADER_SIZE + processed_bytes);
      buf->retrieveAll();
      LOG_DEBUG << "connect finished " << conn->peerAddress().toIpPort();
    } else if (flag == CONNECT2) {
      conn->setContext(CONNECTING);
      auto &tunnels = t_tunnels.value();
      auto tunnel_it = tunnels.find(conn->peerAddress().toIpPort());
      assert_2(tunnel_it != tunnels.end(),
               "tunnel object should be created: client "
                   << conn->peerAddress().toIpPort() << "map: " << &tunnels
                   << "map size: " << tunnels.size());
      tunnel_it->second->connect();
      return;
    } else if (flag == CONNECTING) {
      return;
    } else {
      LOG_FATAL << "unsupport state: " << flag;
    }
  } else {
    const TcpConnectionPtr &clientConn =
        boost::any_cast<const TcpConnectionPtr &>(context);
    clientConn->send(buf);
  }
}

int main() {
  InetAddress listenAddr(1080);
  EventLoop loop;
  g_eventLoop = &loop;
  TcpServer server(&loop, listenAddr, "Socks5");

  server.setConnectionCallback(onServerConnection);
  server.setMessageCallback(onServerMessage);
  server.setThreadNum(6);
  server.start();

  loop.loop();
  return 0;
}