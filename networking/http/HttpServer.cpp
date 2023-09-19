#include "networking/http/HttpServer.h"
#include "networking/TcpServer.h"
#include "networking/http/HttpContext.h"
#include "networking/http/HttpRequest.h"
#include "networking/http/HttpResponse.h"
#include "utility/Logging.h"
#include <boost/any.hpp>

using namespace zhengqi::utility;
using namespace zhengqi::networking;

namespace zhengqi {
namespace networking {
namespace detail {
void defaultHttpCallback(const HttpRequest &, HttpResponse *resp) {
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}
} // namespace detail
} // namespace networking
} // namespace zhengqi

HttpServer::HttpServer(EventLoop *loop, const InetAddress &listenAddr,
                       const std::string &name, TcpServer::Option option)
    : server_(loop, listenAddr, name, option),
      httpCallback_(detail::defaultHttpCallback) {
  server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::start() {
  LOG_WARN << "HttpServer[" << server_.name() << "] starts listening on "
           << server_.ipPort();
  server_.start();
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                           Timestamp receiveTime) {
  HttpContext *context =
      boost::any_cast<HttpContext>(conn->getMutableContext());

  if (!context->parseRequest(buf, receiveTime)) {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  if (context->gotAll()) {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn,
                           const HttpRequest &req) {
  const std::string &connection = req.getHeader("Connection");
  bool close =
      connection == "close" ||
      (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  httpCallback_(req, &response);
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(&buf);
  if (response.closeConnection()) {
    conn->shutdown();
  }
}
