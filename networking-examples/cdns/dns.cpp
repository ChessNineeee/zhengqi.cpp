#include "networking-examples/cdns/Resolver.h"
#include "networking/EventLoop.h"
#include <functional>
#include <stdio.h>

using namespace zhengqi::utility;
using namespace zhengqi::cdns;
using namespace zhengqi::networking;

EventLoop *g_loop;
int count = 0;
int total = 0;

void quit() { g_loop->quit(); }

void resolveCallback(const std::string &host, const InetAddress &addr) {
  printf("resolveCallback %s -> %s\n", host.c_str(), addr.toIpPort().c_str());
  if (++count == total) {
    quit();
  }
}

void resolve(Resolver *res, const std::string &host) {
  res->resolve(host, std::bind(&resolveCallback, host, _1));
}

int main() { return 0; }
