//
// Created by zhengqi on 2023/9/1.
//

#ifndef CPP_CONDITION_H
#define CPP_CONDITION_H

#include "noncopyable.h"
#include "MutexLock.h"

namespace zhengqi
{
    namespace utility
    {
        class Condition : noncopyable{
        public:
            explicit Condition(MutexLock& mutex)
                : mutex_(mutex)
            {
                MCHECK(pthread_cond_init(&pcond_, NULL));
            }

            ~Condition()
            {
                MCHECK(pthread_cond_destroy(&pcond_));
            }

            void wait()
            {
                MutexLock::UnassignGuard ug(mutex_);
                // 3. 所以通过UnassignGuard对象的构造函数将持有者id设置为0
                // 2. 因此互斥量对应的MutexLock的持有者将不再是当前线程
                // 1. pthread_cond_wait 系统调用在阻塞之前会释放互斥量
                MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
                // 1. pthread_cond_wait 系统调用在阻塞返回后会重新获取互斥量
                // 2. 因此互斥量对于的MutexLock的持有者将变成当前线程
                // 3. 所以通过UnassignGuard对象的析构函数将持有者id设置为当前线程id
            }

            // returns true if time out, false otherwise.
            bool waitForSeconds(double seconds);

            void notify()
            {
                MCHECK(pthread_cond_signal(&pcond_));
            }

            void notifyAll()
            {
                MCHECK(pthread_cond_broadcast(&pcond_));
            }

        private:
            MutexLock& mutex_;
            pthread_cond_t pcond_;
        };
    }
}



#endif //CPP_CONDITION_H
