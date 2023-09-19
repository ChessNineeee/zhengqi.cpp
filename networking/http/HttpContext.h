#ifndef ZHENGQI_CPP_NETWORKING_HTTP_HTTPCONTEXT_H
#define ZHENGQI_CPP_NETWORKING_HTTP_HTTPCONTEXT_H

#include "networking/http/HttpRequest.h"
#include "utility/copyable.h"

namespace zhengqi {
namespace networking {
class Buffer;
class HttpContext : public utility::copyable {
public:
  enum HttpRequestParseState {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

  HttpContext() : state_(kExpectRequestLine) {}

  bool parseRequest(Buffer *buf, utility::Timestamp receiveTime);

  bool gotAll() const { return state_ == kGotAll; }

  void reset() {
    state_ = kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
  }

  const HttpRequest &request() const { return request_; }

  HttpRequest &request() { return request_; }

private:
  bool processRequestLine(const char *begin, const char *end);
  HttpRequestParseState state_;
  HttpRequest request_;
};
} // namespace networking
} // namespace zhengqi
#endif // !ZHENGQI_CPP_NETWORKING_HTTP_HTTPCONTEXT_H
