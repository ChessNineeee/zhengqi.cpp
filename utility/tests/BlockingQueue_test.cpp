//
// Created by 70903 on 2023/9/4.
//

#include "utility/BlockingQueue.h"
#include "utility/CountDownLatch.h"
#include "utility/Thread.h"
#include "utility/CurrentThread.h"

#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>

class Test
{
public:
    Test(int numThreads)
        : latch_(numThreads)
    {
        for (int i = 0; i < numThreads; ++i)
        {
            char name[32];
            snprintf(name, sizeof name, "work thread %d", i);
            threads_.emplace_back(new zhengqi::utility::Thread(
                    std::bind(&Test::threadFunc, this), zhengqi::utility::string(name)));
        }
        for (auto& thr : threads_)
        {
            thr->start();
        }
    }

    void run(int times)
    {
        printf("waiting for count down latch\n");
        latch_.wait();
        printf("all threads started\n");
        for (int i = 0; i < times; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof buf, "hello %d", i);
            queue_.put(buf);
            printf("tid=%d, put data = %s, size = %zd\n", zhengqi::utility::CurrentThread::tid(), buf, queue_.size());
        }
    }

    void joinAll()
    {
        for (size_t i = 0; i < threads_.size(); ++i)
        {
            queue_.put("stop");
        }

        for (auto& thr : threads_)
        {
            thr->join();
        }
    }
    
private:
    void threadFunc()
    {
        printf("tid=%d, %s started\n",
               zhengqi::utility::CurrentThread::tid(),
               zhengqi::utility::CurrentThread::name());

        latch_.countDown();
        bool running = true;
        while (running)
        {
            std::string d(queue_.take());
            printf("tid=%d, get data = %s, size = %zd\n", zhengqi::utility::CurrentThread::tid(), d.c_str(), queue_.size());
            running = (d != "stop");
        }

        printf("tid=%d, %s stopped\n",
               zhengqi::utility::CurrentThread::tid(),
               zhengqi::utility::CurrentThread::name());
    }

    zhengqi::utility::BlockingQueue<std::string> queue_;
    zhengqi::utility::CountDownLatch latch_;
    std::vector<std::unique_ptr<zhengqi::utility::Thread>> threads_;
};

void testMove()
{
    zhengqi::utility::BlockingQueue<std::unique_ptr<int>> queue;
    queue.put(std::unique_ptr<int>(new int(42)));
    std::unique_ptr<int> x = queue.take();
    printf("took %d\n", *x);
    *x = 123;
    queue.put(std::move(x));
    std::unique_ptr<int> y = queue.take();
    printf("took %d\n", *y);
}

int main()
{
    printf("pid=%d, tid=%d\n", ::getpid(), zhengqi::utility::CurrentThread::tid());
    Test t(5);
    t.run(100);
    t.joinAll();

    testMove();

    printf("number of created threads %d\n", zhengqi::utility::Thread::numCreated());
    return 0;
}