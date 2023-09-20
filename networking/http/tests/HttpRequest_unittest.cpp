//
// Created by 70903 on 2023/9/20.
//

#define CATCH_CONFIG_MAIN
#include "networking/Buffer.h"
#include "networking/http/HttpContext.h"
#include "utility/Timestamp.h"
#include "utility/tests/catch_amalgamated.h"
#include <string>

using zhengqi::networking::Buffer;
using zhengqi::networking::HttpContext;
using zhengqi::networking::HttpRequest;
using zhengqi::utility::Timestamp;

TEST_CASE("testParseRequestAllInOne") {
  HttpContext context;
  Buffer input;
  input.append("GET /index.html HTTP/1.1\r\n"
               "Host: www.chenshuo.com\r\n"
               "\r\n");
  REQUIRE(context.parseRequest(&input, Timestamp::now()));
  REQUIRE(context.gotAll());
  const HttpRequest &request = context.request();
  REQUIRE(request.method() == HttpRequest::kGet);
  REQUIRE(request.path() == "/index.html");
  REQUIRE(request.getVersion() == HttpRequest::kHttp11);
  REQUIRE(request.getHeader("Host") == "www.chenshuo.com");
  REQUIRE(request.getHeader("User-Agent") == "");
}

TEST_CASE("testParseRequestInTwoPieces") {
  std::string all("GET /index.html HTTP/1.1\r\n"
                  "Host: www.chenshuo.com\r\n"
                  "\r\n");

  for (size_t sz1 = 0; sz1 < all.size(); ++sz1) {
    HttpContext context;
    Buffer input;
    input.append(all.c_str(), sz1);
    REQUIRE(context.parseRequest(&input, Timestamp::now()));
    REQUIRE(!context.gotAll());

    size_t sz2 = all.size() - sz1;
    input.append(all.c_str() + sz1, sz2);
    REQUIRE(context.parseRequest(&input, Timestamp::now()));
    REQUIRE(context.gotAll());
    const HttpRequest &request = context.request();
    REQUIRE(request.method() == HttpRequest::kGet);
    REQUIRE(request.path() == "/index.html");
    REQUIRE(request.getVersion() == HttpRequest::kHttp11);
    REQUIRE(request.getHeader("Host") == "www.chenshuo.com");
    REQUIRE(request.getHeader("User-Agent") == "");
  }
}

TEST_CASE("testParseRequestEmptyHeaderValue") {
  HttpContext context;
  Buffer input;
  input.append("GET /index.html HTTP/1.1\r\n"
               "Host: www.chenshuo.com\r\n"
               "User-Agent:\r\n"
               "Accept-Encoding: \r\n"
               "\r\n");
  REQUIRE(context.parseRequest(&input, Timestamp::now()));
  REQUIRE(context.gotAll());
  const HttpRequest &request = context.request();
  REQUIRE(request.method() == HttpRequest::kGet);
  REQUIRE(request.path() == "/index.html");
  REQUIRE(request.getVersion() == HttpRequest::kHttp11);
  REQUIRE(request.getHeader("Host") == "www.chenshuo.com");
  REQUIRE(request.getHeader("User-Agent") == "");
  REQUIRE(request.getHeader("Accept-Encoding") == "");
}