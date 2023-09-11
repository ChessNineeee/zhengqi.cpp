//
// Created by 70903 on 2023/9/11.
//
#define CATCH_CONFIG_MAIN
#include "utility/tests/catch_amalgamated.h"

#include "networking/Buffer.h"

using namespace zhengqi::networking;
TEST_CASE("testBufferAppendRetrieve") {
  Buffer buf;
  REQUIRE(buf.readableBytes() == 0);
  REQUIRE(buf.writableBytes() == Buffer::kInitialSize);
  REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);

  const std::string str(200, 'x');
  buf.append(str);
  REQUIRE(buf.readableBytes() == str.size());
  REQUIRE(buf.writableBytes() == Buffer::kInitialSize - str.size());
  REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);

  const std::string str2 = buf.retrieveAsString(50);
  REQUIRE(str2.size() == 50);
  REQUIRE(buf.readableBytes() == str.size() - str2.size());
  REQUIRE(buf.writableBytes() == Buffer::kInitialSize - str.size());
  REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend + str2.size());
  REQUIRE(str2 == std::string(50, 'x'));

  buf.append(str);
  REQUIRE(buf.readableBytes() == 2 * str.size() - str2.size());
  REQUIRE(buf.writableBytes() == Buffer::kInitialSize - 2 * str.size());
  REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend + str2.size());

  const std::string str3 = buf.retrieveAllAsString();
  REQUIRE(str3.size() == 350);
  REQUIRE(buf.readableBytes() == 0);
  REQUIRE(buf.writableBytes() == Buffer::kInitialSize);
  REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);
  REQUIRE(str3 == std::string(350, 'x'));
}
