//
// Created by zhengqi on 2023/9/4.
//

#include "utility/ThreadPool.h"
#include "utility/CountDownLatch.h"
#include "utility/CurrentThread.h"

void print()
{
    printf("tid=%d\n", zhengqi::utility::CurrentThread::tid());
}



int main()
{

}