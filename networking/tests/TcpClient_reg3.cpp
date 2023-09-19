#include "networking/EventLoopThread.h"
#include "networking/TcpClient.h"
#include "utility/CurrentThread.h"
#include "utility/Logging.h"

using namespace zhengqi::utility;
using namespace zhengqi::networking;

int main() {
  Logger::setLogLevel(Logger::DEBUG);
  EventLoopThread loopThread;
  {
    InetAddress serverAddr("127.0.0.1", 1234);
    TcpClient client(loopThread.startLoop(), serverAddr, "TcpClient");
    client.connect();
    CurrentThread::sleepUsec(500 * 1000);
    client.disconnect();
  }
  CurrentThread::sleepUsec(1000 * 1000);
}
