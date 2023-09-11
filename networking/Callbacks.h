#ifndef ZHENGQI_CPP_NETWORKING_CALLBACKS_H
#define ZHENGQI_CPP_NETWORKING_CALLBACKS_H

#include "utility/Timestamp.h"

#include <cstddef>
#include <functional>
#include <memory>

namespace zhengqi {
namespace utility {
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// namespace utility

template <typename T> inline T *get_pointer(const std::shared_ptr<T> &ptr) {
  return ptr.get();
}

template <typename T> inline T *get_pointer(const std::unique_ptr<T> &ptr) {
  return ptr.get();
}

template <typename To, typename From>
inline ::std::shared_ptr<To>
down_pointer_cast(const ::std::shared_ptr<From> &f) {
#ifndef NDEBUG

  assert(f == NULL || dynamic_cast<To *>(get_pointer(f)) != NULL);

#endif // !DEBUG
  return ::std::static_pointer_cast<To>(f);
}

} // namespace utility

namespace networking {
class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr &, size_t)>
    HighWaterMarkCallback;

typedef std::function<void(const TcpConnectionPtr &, Buffer *,
                           utility::Timestamp)>
    MessageCallback;

void defaultConnectionCallback(const TcpConnectionPtr &conn);
void defaultMessageCallback(const TcpConnectionPtr &conn, Buffer *buffer,
                            utility::Timestamp receiveTime);
} // namespace networking

} // namespace zhengqi

#endif // !DEBUG
