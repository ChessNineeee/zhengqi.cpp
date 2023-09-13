#ifndef ZHENGQI_CPP_NETWORKING_TCPCLIENT_H
#define ZHENGQI_CPP_NETWORKING_TCPCLIENT_H

#include "networking/Callbacks.h"
#include "networking/TcpConnection.h"
#include "utility/MutexLock.h"
#include "utility/noncopyable.h"
#include <algorithm>
#include <memory>

namespace zhengqi {
namespace networking {

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : utility::noncopyable {
public:
  TcpClient(EventLoop *loop, const InetAddress &serverAddr,
            const std::string &nameArg);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    utility::MutexLockGuard lock(mutex_);
    return connection_;
  }

  EventLoop *getLoop() const { return loop_; }
  bool retry() const { return retry_; }
  void enableRetry() { retry_ = true; }

  const std::string &name() const { return name_; }

  /// Set connection callback
  /// Not thread safe
  void setConnectionCallback(ConnectionCallback cb) {
    connectionCallback_ = std::move(cb);
  }

  /// Set message callback
  /// Not thread safe
  void setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
  }

  /// Set write complete callback
  /// Not thread safe
  void setWriteCompleteCallback(WriteCompleteCallback cb) {
    writeCompleteCallback_ = std::move(cb);
  }

private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd);
  /// Not thread safe, but in loop
  void removeConnection(const TcpConnectionPtr &conn);

  EventLoop *loop_;
  ConnectorPtr connector_;
  const std::string name_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool retry_;   // atomic
  bool connect_; // atomic
  // always in loop thread
  int nextConnId_;
  mutable utility::MutexLock mutex_;
  TcpConnectionPtr connection_;
};
} // namespace networking
} // namespace zhengqi

#endif // !DEBUG
