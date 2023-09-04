//
// Created by zhengqi on 2023/9/2.
//
#include "utility/CountDownLatch.h"
#include <thread>
#include <iostream>
#include <unistd.h>

int main()
{
    zhengqi::utility::CountDownLatch cdl(10);
    std::thread t1([&](){
        for (int i = 0; i < 5; i++)
        {
            std::cout << "a: " << cdl.getCount() << std::endl;
            sleep(1);
            cdl.countDown();
        }
    }), t2([&](){
        for (int i = 0; i < 5; i++)
        {
            std::cout << "b: " << cdl.getCount() << std::endl;
            sleep(1);
            cdl.countDown();
        }
    });

    cdl.wait();
    std::cout << "c: " << cdl.getCount() << std::endl;
    return 0;
}
