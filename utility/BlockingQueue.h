//
// Created by 70903 on 2023/9/4.
//

#ifndef CPP_BLOCKINGQUEUE_H
#define CPP_BLOCKINGQUEUE_H

#include "utility/noncopyable.h"
#include "utility/Condition.h"
#include "utility/MutexLock.h"

#include <deque>
#include <assert.h>

namespace zhengqi
{
    namespace utility
    {
        template<typename T>
        class BlockingQueue : noncopyable
        {
        public:
            using queue_type = std::deque<T>;
            BlockingQueue()
            : mutexLock_(),
              notEmpty_(mutexLock_),
              queue_()
            {}

            void put(const T& x)
            {
                MutexLockGuard lockGuard(mutexLock_);
                queue_.push_back(x);
                notEmpty_.notify();
            }

            void put(T&& x)
            {
                MutexLockGuard lock(mutexLock_);
                queue_.push_back(std::move(x));
                notEmpty_.notify();
            }

            T take()
            {
                MutexLockGuard lock(mutexLock_);
                while (queue_.empty())
                {
                    notEmpty_.wait();
                }
                assert(!queue_.empty());
                T front(std::move(queue_.front()));
                queue_.pop_front();
                return front;
            }

            queue_type drain()
            {
                std::deque<T> queue;
                {
                    MutexLockGuard lockGuard(mutexLock_);
                    queue = std::move(queue_);
                    assert(queue_.empty());
                }
                return queue;
            }

            size_t size() const
            {
                MutexLockGuard lockGuard(mutexLock_);
                return queue_.size();
            }
        private:
            mutable MutexLock mutexLock_;
            Condition notEmpty_;
            queue_type queue_;
        };
    }
}

#endif //CPP_BLOCKINGQUEUE_H
