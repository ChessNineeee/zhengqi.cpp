#include "networking-examples/socks4a/tunnel.h"
#include "networking/Callbacks.h"
#include "networking/InetAddress.h"
#include "utility/Logging.h"
#include "utility/ThreadLocal.h"
#include <boost/any.hpp>
#include <cassert>
#include <vector>

using namespace zhengqi::utility;
using namespace zhengqi::networking;

std::vector<InetAddress> g_backends;
ThreadLocal<std::map<std::string, TunnelPtr>> t_tunnels;
MutexLock g_mutex;
size_t g_current = 0;

void onServerConnection(const TcpConnectionPtr &conn) {
  LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
  std::map<std::string, TunnelPtr> &tunnels = t_tunnels.value();
  if (conn->connected()) {
    conn->setTcpNoDelay(true);
    conn->stopRead();
    size_t current = 0;
    {
      MutexLockGuard guard(g_mutex);
      current = g_current;
      g_current = (g_current + 1) % g_backends.size();
    }

    InetAddress backend = g_backends[current];
    TunnelPtr tunnel(new Tunnel(conn->getLoop(), backend, conn));
    tunnel->setup();
    tunnel->connect();

    tunnels[conn->name()] = tunnel;
  } else {
    assert(tunnels.find(conn->name()) != tunnels.end());
    tunnels[conn->name()]->disconnect();
    tunnels.erase(conn->name());
  }
}

void onServerMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp) {
  if (!conn->getContext().empty()) {
    const TcpConnectionPtr &clientConn =
        boost::any_cast<const TcpConnectionPtr &>(conn->getContext());
    clientConn->send(buf);
  }
}
