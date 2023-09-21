#ifndef ZHENGQI_CPP_NETWORKING_EXAMPLES_CDNS_RESOLVER_H
#define ZHENGQI_CPP_NETWORKING_EXAMPLES_CDNS_RESOLVER_H

#include "networking/InetAddress.h"
#include "utility/StringPiece.h"
#include "utility/Timestamp.h"
#include "utility/noncopyable.h"

#include <functional>
#include <map>
#include <memory>

extern "C" {
struct hostent;
struct ares_channeldata;
typedef struct ares_channeldata *ares_channel;
}

namespace zhengqi {
namespace networking {

class Channel;
class EventLoop;
} // namespace networking
//
namespace cdns {
class Resolver : utility::noncopyable {
public:
  typedef std::function<void(const networking::InetAddress &)> Callback;
  enum Option {
    kDNSandHostsFile,
    kDNSonly,
  };
  explicit Resolver(networking::EventLoop *loop, Option opt = kDNSandHostsFile);

  ~Resolver();

  bool resolve(utility::StringArg hostname, const Callback &cb);

private:
  struct QueryData {
    Resolver *owner;
    Callback callback;
    QueryData(Resolver *o, const Callback &cb) : owner(o), callback(cb) {}
  };

  networking::EventLoop *loop_;
  ares_channel ctx_;
  bool timerActive_;

  typedef std::map<int, std::unique_ptr<networking::Channel>> ChannelList;
  ChannelList channels_;

  void onRead(int sockfd, utility::Timestamp t);
  void onTimer();
  void onQueryResult(int status, struct hostent *result, const Callback &cb);
  void onSockCreate(int sockfd, int type);
  void onSockStateChange(int sockfd, bool read, bool write);

  static void ares_host_callback(void *data, int status, int timeouts,
                                 struct hostent *hostent);
  static int ares_sock_create_callback(int sockfd, int type, void *data);
  static void ares_sock_state_callback(void *data, int sockfd, int read,
                                       int write);
};
} // namespace cdns
} // namespace zhengqi
#endif // !DEBUG
