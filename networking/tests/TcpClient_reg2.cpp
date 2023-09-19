#include "networking/EventLoop.h"
#include "networking/TcpClient.h"
#include "utility/CurrentThread.h"
#include "utility/Logging.h"
#include "utility/Thread.h"
#include <functional>

using namespace zhengqi::utility;
using namespace zhengqi::networking;

void threadFunc(EventLoop *loop) {
  InetAddress serverAddr("127.0.0.1", 1234);
  TcpClient client(loop, serverAddr, "TcpClient");
  client.connect();

  CurrentThread::sleepUsec(1000 * 1000);
}

int main(int argc, char *argv[]) {
  EventLoop loop;
  loop.runAfter(3.0, std::bind(&EventLoop::quit, &loop));
  Thread thr(std::bind(threadFunc, &loop));
  thr.start();
  loop.loop();
}
