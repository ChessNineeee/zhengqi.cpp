//
// Created by zhengqi on 2023/9/4.
//

#include "utility/CountDownLatch.h"
#include "utility/CurrentThread.h"
#include "utility/Logging.h"
#include "utility/ThreadPool.h"
#include <cstdio>
#include <functional>
#include <string>
#include <unistd.h>

void print() { printf("tid=%d\n", zhengqi::utility::CurrentThread::tid()); }

void printString(const std::string &str) {
  LOG_INFO << str;
  usleep(100 * 1000);
}

void test(int maxSize) {
  LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
  zhengqi::utility::ThreadPool pool("MainThreadPool");
  pool.setMaxQueueSize(maxSize);
  pool.start(5);

  LOG_WARN << "Adding";
  pool.run(print);
  pool.run(print);

  for (int i = 0; i < 100; i++) {
    char buf[32];
    snprintf(buf, sizeof buf, "task %d", i);
    pool.run(std::bind(printString, std::string(buf)));
  }
  LOG_WARN << "Done";

  zhengqi::utility::CountDownLatch latch(1);
  pool.run(std::bind(&zhengqi::utility::CountDownLatch::countDown, &latch));
  latch.wait();
  pool.stop();
}

void longTask(int num) {
  LOG_INFO << "longTask " << num;
  zhengqi::utility::CurrentThread::sleepUsec(3000000);
}

void test2() {
  LOG_WARN << "Test ThreadPool by stoping early.";
  zhengqi::utility::ThreadPool pool("ThreadPool");
  pool.setMaxQueueSize(5);
  pool.start(3);

  zhengqi::utility::Thread thread1(
      [&pool]() {
        for (int i = 0; i < 20; ++i) {
          pool.run(std::bind(longTask, i));
        }
      },
      "thread1");
  thread1.start();

  zhengqi::utility::CurrentThread::sleepUsec(5000000);
  LOG_WARN << "stop pool";
  pool.stop(); // early stop

  thread1.join();
  // run() after stop()
  pool.run(print);
  LOG_WARN << "test2 Done";
}

int main() {
  test(0);
  test(1);
  test(5);
  test(10);
  test(50);
  test2();
}
