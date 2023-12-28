#include "networking-examples/socks4a/tunnel.h"
#include "networking/Callbacks.h"
#include "networking/Endian.h"
#include "networking/EventLoop.h"
#include "networking/TcpServer.h"
#include "utility/CurrentThread.h"
#include "utility/Types.h"
#include <algorithm>
#include <boost/any.hpp>
#include <map>
#include <netdb.h>
#include <stdio.h>
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

using namespace zhengqi::utility;
using namespace zhengqi::networking;

EventLoop *g_eventLoop;
std::unordered_set<string> g_handshake;
std::unordered_set<string> g_connect;
std::map<string, TunnelPtr> g_tunnels;

void onServerConnection(const TcpConnectionPtr &conn) {
  assert(conn != nullptr);
  LOG_DEBUG << conn->peerAddress().toIpPort()
            << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected()) {
    conn->setTcpNoDelay(true);
    assert(g_handshake.find(conn->peerAddress().toIpPort()) ==
           g_handshake.end());
    assert(g_connect.find(conn->peerAddress().toIpPort()) == g_connect.end());
    g_handshake.insert(conn->peerAddress().toIpPort());
  } else {
    std::map<string, TunnelPtr>::iterator it =
        g_tunnels.find(conn->peerAddress().toIpPort());
    if (it != g_tunnels.end()) {
      it->second->disconnect();
      g_tunnels.erase(it);
    }
  }
}

void onServerMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp) {
  assert(conn != nullptr && buf != nullptr);
  LOG_DEBUG << conn->peerAddress().toIpPort() << " " << buf->readableBytes();

  if (g_handshake.find(conn->peerAddress().toIpPort()) != g_handshake.end()) {
    assert(g_connect.find(conn->peerAddress().toIpPort()) == g_connect.end());
    assert(buf->readableBytes() <= 3);
    if (buf->readableBytes() < 3)
      return;

    LOG_DEBUG << conn->peerAddress().toIpPort() << " first handshake";
    assert(buf->peek()[0] == SOCKS5_VERSION);
    assert(buf->peek()[1] == 0x01);
    assert(buf->peek()[2] == NOAUTH);
    buf->retrieve(3);

    char reply[2] = {SOCKS5_VERSION, NOAUTH};
    conn->send(reply, 2);

    g_handshake.erase(conn->peerAddress().toIpPort());
    g_connect.insert(conn->peerAddress().toIpPort());
  } else if (g_connect.find(conn->peerAddress().toIpPort()) !=
             g_connect.end()) {
    assert(g_handshake.find(conn->peerAddress().toIpPort()) ==
           g_handshake.end());
    assert(buf->readableBytes() < DOMAIN_SIZE_LIMIT);

    LOG_DEBUG << conn->peerAddress().toIpPort() << " connect remote";
    assert(buf->peek()[0] == SOCKS5_VERSION);
    assert(buf->peek()[1] == SOCKS5_CONNECT);
    assert(buf->peek()[2] == SOCKS5_RSV);

    char host_name_type = buf->peek()[3];
    assert(host_name_type == SOCKS5_HOST_IN_IPv4 ||
           host_name_type == SOCKS5_HOST_IN_DOMAIN);

    sockaddr_in remote_addr;
    memZero(&remote_addr, sizeof remote_addr);
    remote_addr.sin_family = AF_INET;

    size_t processed_bytes = 0;
    if (host_name_type == SOCKS5_HOST_IN_IPv4) {
      size_t connect_bytes_len = SOCKS5_CONNECT_HEADER_SIZE + IP_ADDR_SIZE + 2;
      assert(buf->readableBytes() <= connect_bytes_len);
      if (buf->readableBytes() < connect_bytes_len)
        return;
      processed_bytes = IP_ADDR_SIZE + 2;
      const void *remote_ip = buf->peek() + SOCKS5_CONNECT_HEADER_SIZE;
      const void *remote_port =
          buf->peek() + IP_ADDR_SIZE + SOCKS5_CONNECT_HEADER_SIZE;
      remote_addr.sin_port = *static_cast<const in_port_t *>(remote_port);
      remote_addr.sin_addr.s_addr = *static_cast<const uint32_t *>(remote_ip);
    } else {
      const void *host_name_len_ptr = buf->peek() + SOCKS5_CONNECT_HEADER_SIZE;
      const int8_t host_name_len =
          *static_cast<const int8_t *>(host_name_len_ptr);
      size_t connect_bytes_len =
          SOCKS5_CONNECT_HEADER_SIZE + host_name_len + 2 + 1;

      assert(buf->readableBytes() <= connect_bytes_len);
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
        assert(false);
      }
    }

    InetAddress remote_iaddr(remote_addr);
    LOG_DEBUG << "connect to Remote: " << remote_iaddr.toIpPort();
    TunnelPtr tunnel(new Tunnel(g_eventLoop, remote_iaddr, conn));
    tunnel->setup();
    tunnel->connect();
    g_tunnels[conn->peerAddress().toIpPort()] = tunnel;

    char reply[4] = {SOCKS5_VERSION, SOCKS5_CONNECT_OK, SOCKS5_RSV,
                     host_name_type};
    conn->send(reply, 4);
    conn->send(buf->peek() + SOCKS5_CONNECT_HEADER_SIZE,
               static_cast<int>(processed_bytes));

    assert(buf->readableBytes() ==
           SOCKS5_CONNECT_HEADER_SIZE + processed_bytes);
    buf->retrieveAll();

    g_connect.erase(conn->peerAddress().toIpPort());
  } else if (!conn->getContext().empty()) {
    const TcpConnectionPtr &clientConn =
        boost::any_cast<const TcpConnectionPtr &>(conn->getContext());
    clientConn->send(buf);
  } else {
    assert(false);
  }
}

int main() {
  InetAddress listenAddr(1080);
  EventLoop loop;
  g_eventLoop = &loop;
  TcpServer server(&loop, listenAddr, "Socks5");

  server.setConnectionCallback(onServerConnection);
  server.setMessageCallback(onServerMessage);

  server.start();

  loop.loop();
  return 0;
}