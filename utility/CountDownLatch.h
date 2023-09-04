//
// Created by zhengqi on 2023/9/2.
//

#ifndef CPP_COUNTDOWNLATCH_H
#define CPP_COUNTDOWNLATCH_H

#include "noncopyable.h"
#include "MutexLock.h"
#include "Condition.h"

namespace zhengqi
{
    namespace utility
    {
        class CountDownLatch : noncopyable {
        public:
            explicit CountDownLatch(int count);

            void wait();

            void countDown();

            int getCount() const;
        private:
            mutable MutexLock mutex_;
            Condition condition_;
            int count_;
        };
    }
}




#endif //CPP_COUNTDOWNLATCH_H
