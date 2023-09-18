#ifndef ZHENGQI_CPP_NETWORKING_TCPSERVER_H
#define ZHENGQI_CPP_NETWORKING_TCPSERVER_H

#include "networking/TcpConnection.h"
#include "utility/Atomic.h"
#include "utility/noncopyable.h"
#include <functional>
#include <map>
#include <memory>
namespace zhengqi {
namespace networking {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : utility::noncopyable {
public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;
  enum Option {
    kNoReusePort,
    kReusePort,
  };

  TcpServer(EventLoop *loop, const InetAddress &listenAddr,
            const std::string &nameArg, Option option = kNoReusePort);

  ~TcpServer();

  const std::string &ipPort() const { return ipPort_; }
  const std::string &name() const { return name_; }
  EventLoop *getLoop() const { return loop_; }

  void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback &cb) {
    threadInitCallback_ = cb;
  }

  std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

  void start();

  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }

private:
  void newConnection(int sockfd, const InetAddress &peerAddr);
  void removeConnection(const TcpConnectionPtr &conn);
  void removeConnectionInLoop(const TcpConnectionPtr &conn);

  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
  EventLoop *loop_;
  const std::string ipPort_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> threadPool_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  ThreadInitCallback threadInitCallback_;
  utility::AtomicInt32 started_;
  int nextConnId_;
  ConnectionMap connections_;
};

} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_TCPSERVER_H
