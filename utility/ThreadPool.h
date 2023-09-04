//
// Created by zhengqi on 2023/9/3.
//

#ifndef CPP_THREADPOOL_H
#define CPP_THREADPOOL_H

#include <functional>
#include <vector>
#include <deque>
#include "Types.h"
#include "noncopyable.h"
#include "MutexLock.h"
#include "Condition.h"
#include "Thread.h"

namespace zhengqi {
    namespace utility {

        class ThreadPool : noncopyable
        {
        public:
            typedef std::function<void ()> Task;

            explicit ThreadPool(const string& nameArg = string("ThreadPool"));
            ~ThreadPool();

            void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
            void setThreadInitCallback(const Task& cb)
            {
                threadInitCallback_ = cb;
            }

            void start(int numThreads);
            void stop();
            const string& name() const
            {
                return name_;
            }
            size_t queueSize() const;
            // Could block if maxQueueSize > 0
            // Call after stop() will return immediately.
            // There is no move-only version of std::function in C++ as of C++14.
            // So we don't need to overload a const& and an && versions
            // as we do in (Bounded)BlockingQueue.
            // https://stackoverflow.com/a/25408989
            void run(Task f);
        private:
            bool isFull() const;
            void runInThread();
            Task take();

            mutable MutexLock mutex_;
            Condition notEmpty_; // GUARDED_BY(mutex_);
            Condition notFull_; // GUARDED_BY(mutex_);
            string name_;
            Task threadInitCallback_;
            std::vector<std::unique_ptr<Thread>> threads_;
            std::deque<Task> queue_;
            size_t maxQueueSize_;
            bool running_;
        };

    } // zhengqi
} // utility

#endif //CPP_THREADPOOL_H
