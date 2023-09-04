//
// Created by zhengqi on 2023/9/3.
//

#ifndef CPP_THREAD_H
#define CPP_THREAD_H

#include <functional>
#include <string>
#include <memory>
#include "noncopyable.h"
#include "CountDownLatch.h"
#include "Atomic.h"

namespace zhengqi {
    namespace utility {

        class Thread : noncopyable {
        public:
            typedef std::function<void ()> ThreadFunc;

            explicit Thread(ThreadFunc, const std::string& name = std::string());

            ~Thread();

            void start();
            int join(); // return pthread_join()

            bool started() const { return started_; }
            pid_t tid() const { return tid_; }
            const std::string& name() const { return name_; }

            static int numCreated() { return numCreated_.get(); }
        private:
            void setDefaultName();

            bool started_;
            bool joined_;
            pthread_t pthreadId_;
            pid_t tid_;
            ThreadFunc func_;
            std::string name_;
            CountDownLatch latch_;

            static AtomicInt32 numCreated_;
        };

    } // zhengqi
} // utility

#endif //CPP_THREAD_H
