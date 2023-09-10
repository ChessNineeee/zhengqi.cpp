//
// Created by 70903 on 2023/9/10.
//
#define CATCH_CONFIG_MAIN
#include "utility/tests/catch_amalgamated.h"

#include "networking/InetAddress.h"
#include "utility/Logging.h"

using namespace zhengqi::utility;
using namespace zhengqi::networking;

TEST_CASE("testInetAddress") {
  InetAddress addr0(1234);
  REQUIRE(addr0.toIp() == string("0.0.0.0"));
  REQUIRE(addr0.toIpPort() == string("0.0.0.0:1234"));
  REQUIRE(addr0.port() == 1234);

  InetAddress addr1(4321, true);
  REQUIRE(addr1.toIp() == string("127.0.0.1"));
  REQUIRE(addr1.toIpPort() == string("127.0.0.1:4321"));
  REQUIRE(addr1.port() == 4321);

  InetAddress addr2("1.2.3.4", 8888);
  REQUIRE(addr2.toIp() == string("1.2.3.4"));
  REQUIRE(addr2.toIpPort() == string("1.2.3.4:8888"));
  REQUIRE(addr2.port() == 8888);

  InetAddress addr3("255.254.253.252", 65535);
  REQUIRE(addr3.toIp() == string("255.254.253.252"));
  REQUIRE(addr3.toIpPort() == string("255.254.253.252:65535"));
  REQUIRE(addr3.port() == 65535);
}

TEST_CASE("testInet6Address") {
  InetAddress addr0(1234, false, true);
  REQUIRE(addr0.toIp() == string("::"));
  REQUIRE(addr0.toIpPort() == string("[::]:1234"));
  REQUIRE(addr0.port() == 1234);

  InetAddress addr1(1234, true, true);
  REQUIRE(addr1.toIp() == string("::1"));
  REQUIRE(addr1.toIpPort() == string("[::1]:1234"));
  REQUIRE(addr1.port() == 1234);
  InetAddress addr2("2001:db8::1", 8888, true);
  REQUIRE(addr2.toIp() == string("2001:db8::1"));
  REQUIRE(addr2.toIpPort() == string("[2001:db8::1]:8888"));
  REQUIRE(addr2.port() == 8888);
  InetAddress addr3("fe80::1234:abcd:1", 8888);
  REQUIRE(addr3.toIp() == string("fe80::1234:abcd:1"));
  REQUIRE(addr3.toIpPort() == string("[fe80::1234:abcd:1]:8888"));
  REQUIRE(addr3.port() == 8888);
}

TEST_CASE("testInetAddressResolve") {
  InetAddress addr(80);
  if (InetAddress::resolve("baidu.com", &addr)) {
    LOG_INFO << "baidu.com resolved to " << addr.toIpPort();
  } else {
    LOG_ERROR << "Unable to resolve baidu.com";
  }
}
