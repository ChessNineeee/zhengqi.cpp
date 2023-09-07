#include "utility/ThreadLocalSingleton.h"
#include "utility/CurrentThread.h"
#include "utility/Thread.h"

#include <stdio.h>

class Test : zhengqi::utility::noncopyable {
public:
    Test() {
        printf("tid=%d, constructing %p\n", zhengqi::utility::CurrentThread::tid(),
               this);
    }

    ~Test() {
        printf("tid=%d, destructing %p %s\n",
               zhengqi::utility::CurrentThread::tid(), this, name_.c_str());
    }

    const zhengqi::utility::string &name() const { return name_; }

    void setName(const zhengqi::utility::string &n) { name_ = n; }

private:
    zhengqi::utility::string name_;
};

void threadFunc(const char *changeTo) {
    // 所有线程使用同一个ThreadLocalSingleton对象
    // 映射关系为：tid ---> Test对象
    printf("tid=%d, %p name=%s\n",
           zhengqi::utility::CurrentThread::tid(),
           &zhengqi::utility::ThreadLocalSingleton<Test>::instance(),
           zhengqi::utility::ThreadLocalSingleton<Test>::instance().name().c_str());
    zhengqi::utility::ThreadLocalSingleton<Test>::instance().setName(changeTo);
    printf("tid=%d, %p name=%s\n",
           zhengqi::utility::CurrentThread::tid(),
           &zhengqi::utility::ThreadLocalSingleton<Test>::instance(),
           zhengqi::utility::ThreadLocalSingleton<Test>::instance().name().c_str());
}

int main() {
    zhengqi::utility::ThreadLocalSingleton<Test>::instance().setName("main one");
    zhengqi::utility::Thread t1(std::bind(threadFunc, "thread1"));
    zhengqi::utility::Thread t2(std::bind(threadFunc, "thread2"));
    t1.start();
    t2.start();
    t1.join();
    printf("tid=%d, %p name=%s\n",
           zhengqi::utility::CurrentThread::tid(),
           &zhengqi::utility::ThreadLocalSingleton<Test>::instance(),
           zhengqi::utility::ThreadLocalSingleton<Test>::instance().name().c_str());
    t2.join();

    pthread_exit(0);
}