#include "networking/EventLoop.h"
#include "networking/TcpClient.h"
#include "utility/CurrentThread.h"
#include "utility/Logging.h"

using namespace zhengqi::utility;
using namespace zhengqi::networking;

TcpClient *g_client;

void timeout() {
  LOG_INFO << "timeout";
  g_client->stop();
}

int main() {
  Logger::setLogLevel(Logger::DEBUG);
  EventLoop loop;
  InetAddress serverAddr("127.0.0.1", 2);
  TcpClient client(&loop, serverAddr, "TcpClient");
  g_client = &client;
  loop.runAfter(0.0, timeout);
  loop.runAfter(1.0, std::bind(&EventLoop::quit, &loop));
  client.connect();
  CurrentThread::sleepUsec(100 * 1000);
  loop.loop();
}
