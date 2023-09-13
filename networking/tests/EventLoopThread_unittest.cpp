//
// Created by 70903 on 2023/9/13.
//

#include "networking/EventLoop.h"
#include "networking/EventLoopThread.h"
#include "utility/CountDownLatch.h"
#include "utility/Thread.h"

using namespace zhengqi::utility;
using namespace zhengqi::networking;

void print(EventLoop *p = NULL) {
  printf("print: pid = %d, tid = %d, loop = %p\n", getpid(),
         CurrentThread::tid(), p);
}

void quit(EventLoop *p) {
  print(p);
  p->quit();
}

int main() {
  print();
  { EventLoopThread thr1; }
  {
    EventLoopThread thr2;
    EventLoop *loop = thr2.startLoop();
    loop->runInLoop(std::bind(print, loop));
    CurrentThread::sleepUsec(500 * 1000);
  }
  {
    EventLoopThread thr3;
    EventLoop *loop = thr3.startLoop();
    loop->runInLoop(std::bind(quit, loop));
    CurrentThread::sleepUsec(500 * 1000);
  }
}