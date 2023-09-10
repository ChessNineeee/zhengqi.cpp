#ifndef ZHENGQI_CPP_NETWORKING_INETADDRESS_H
#define ZHENGQI_CPP_NETWORKING_INETADDRESS_H

#include "utility/StringPiece.h"
#include "utility/copyable.h"
#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>

namespace zhengqi {
namespace networking {
namespace sockets {
const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr);
}

class InetAddress : public utility::copyable {
public:
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false,
                       bool ipv6 = false);

  InetAddress(utility::StringArg ip, uint16_t port, bool ipv6 = false);

  explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

  explicit InetAddress(const struct sockaddr_in6 &addr) : addr6_(addr) {}

  sa_family_t family() const { return addr_.sin_family; }

  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t port() const;

  const struct sockaddr *getSockAddr() const {
    return sockets::sockaddr_cast(&addr6_);
  }

  uint32_t ipv4NetEndian() const;
  uint16_t portNetEndian() const;

  static bool resolve(utility::StringArg hostname, InetAddress *result);

  void setScopeId(uint32_t scope_id);

private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};
} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_INETADDRESS_H
