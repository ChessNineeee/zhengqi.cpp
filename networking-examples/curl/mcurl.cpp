#include "networking-examples/curl/Curl.h"
#include "networking/EventLoop.h"

using namespace zhengqi::networking;
using namespace zhengqi::curl;

EventLoop *g_loop = NULL;

void onData(const char *data, int len) {
  printf("data %s\nlen %d\n", data, len);
}

void done(Request *c, int code) {
  printf("done %p %s %d\n", c, c->getEffectiveUrl(), code);
}

void done2(Request *c, int code) {
  printf("done2 %p %s %d %d\n", c, c->getRedirectUrl(), c->getResponseCode(),
         code);
}

int main(int argc, char *argv[]) {
  EventLoop loop;
  g_loop = &loop;
  loop.runAfter(30.0, std::bind(&EventLoop::quit, &loop));
  Curl::initialize(Curl::kCURLssl);
  Curl curl(&loop);
  RequestPtr req = curl.getUrl("http://chenshuo.com");

  req->setDataCallback(onData);
  req->setDoneCallback(done);

  RequestPtr req2 = curl.getUrl("http://baidu.com");
  req2->setDataCallback(onData);
  req2->setDoneCallback(done2);

  RequestPtr req3 = curl.getUrl("http://nba.hupu.com");
  req3->setDataCallback(onData);
  req3->setDoneCallback(done2);

  loop.loop();
}
