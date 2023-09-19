#ifndef ZHENGQI_CPP_NETWORKING_HTTP_HTTPSERVER_H
#define ZHENGQI_CPP_NETWORKING_HTTP_HTTPSERVER_H

#include "networking/Callbacks.h"
#include "networking/TcpServer.h"
#include "utility/noncopyable.h"
#include <functional>
#include <string>

namespace zhengqi {
namespace networking {

class HttpRequest;
class HttpResponse;

class HttpServer : utility::noncopyable {
public:
  typedef std::function<void(const HttpRequest &, HttpResponse *)> HttpCallback;

  HttpServer(EventLoop *loop, const InetAddress &listenAddr,
             const std::string &name,
             TcpServer::Option option = TcpServer::kNoReusePort);

  EventLoop *getLoop() const { return server_.getLoop(); }

  void setHttpCallback(const HttpCallback &cb) { httpCallback_ = cb; }

  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

  void start();

private:
  void onConnection(const TcpConnectionPtr &conn);
  void onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                 utility::Timestamp receiveTime);
  void onRequest(const TcpConnectionPtr &, const HttpRequest &);

  TcpServer server_;
  HttpCallback httpCallback_;
};

} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_HTTP_HTTPSERVER_H
