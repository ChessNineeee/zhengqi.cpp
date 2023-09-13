#ifndef ZHENGQI_CPP_NETWORKING_TCPCLIENT_H
#define ZHENGQI_CPP_NETWORKING_TCPCLIENT_H

#include "networking/TcpConnection.h"
#include "utility/MutexLock.h"
#include <memory>

namespace zhengqi {
namespace networking {

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;
} // namespace networking
} // namespace zhengqi

#endif // !DEBUG
