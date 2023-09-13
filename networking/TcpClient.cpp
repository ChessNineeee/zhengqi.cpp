#include "networking/TcpClient.h"
#include "networking/Callbacks.h"
#include "networking/Connector.h"
#include "networking/EventLoop.h"
#include "networking/InetAddress.h"
#include "networking/SocketsOps.h"
#include "utility/Logging.h"
#include "utility/MutexLock.h"
#include <cassert>
#include <functional>

using namespace zhengqi::utility;
using namespace zhengqi::networking;

namespace zhengqi {
namespace networking {
namespace detail {
void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn) {
  loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr &connector) {}
} // namespace detail
} // namespace networking
} // namespace zhengqi

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr,
                     const std::string &nameArg)
    : loop_(CHECK_NOTNULL(loop)), connector_(new Connector(loop, serverAddr)),
      name_(nameArg), connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback), retry_(false), connect_(true),
      nextConnId_(1) {
  connector_->setNewConnectionCallback(
      std::bind(&TcpClient::newConnection, this, _1));
  LOG_INFO << "TcpClient::TcpClient[" << name_ << "] - connector "
           << get_pointer(connector_);
}

TcpClient::~TcpClient() {
  LOG_INFO << "TcpClient::~TcpClient[" << name_ << "] - connector "
           << get_pointer(connector_);

  TcpConnectionPtr conn;
  bool unique = false;

  {
    MutexLockGuard lock(mutex_);
    unique = connection_.unique();
    conn = connection_;
  }

  if (conn) {
    assert(loop_ == conn->getLoop());
    CloseCallback cb = std::bind(&detail::removeConnection, loop_, _1);
    loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));

    if (unique) {
      conn->forceClose();
    }
  } else {
    connector_->stop();
    loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
  }
}

void TcpClient::connect() {
  LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
           << connector_->serverAddress().toIpPort();
  connect_ = true;
  connector_->start();
}
