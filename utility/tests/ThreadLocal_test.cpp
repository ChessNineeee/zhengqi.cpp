#include "utility/CurrentThread.h"
#include "utility/Thread.h"
#include "utility/ThreadLocal.h"
#include "utility/Types.h"
#include "utility/noncopyable.h"

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

zhengqi::utility::ThreadLocal<Test> testObj1;
zhengqi::utility::ThreadLocal<Test> testObj2;

void print() {
    printf("tid=%d, obj1 %p name=%s\n",
           zhengqi::utility::CurrentThread::tid(),
           &testObj1.value(), testObj1.value().name().c_str());
    printf("tid=%d, obj2 %p name=%s\n",
           zhengqi::utility::CurrentThread::tid(),
           &testObj2.value(), testObj2.value().name().c_str());
}

void threadFunc() {
    // 创建自己的ThreadLocal的Test对象
    // 映射关系为(tid, pkey) ---> Test
    print();
    testObj1.value().setName("changed 1");
    testObj2.value().setName("changed 42");
    print();
}

int main() {
    testObj1.value().setName("main one");
    print();
    zhengqi::utility::Thread t1(threadFunc);
    t1.start();
    t1.join();
    testObj2.value().setName("main two");
    print();

    pthread_exit(0);
}