//
// Created by zhengqi on 2023/9/3.
//

#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
#include "Thread.h"
#include "Exception.h"
#include "Timestamp.h"

namespace zhengqi {
    namespace utility {
        pid_t gettid()
        {
            // task_struct id
            // gettid 在 macos 下的实现和 在 linux 下的实现完全无关
            // linux 下 可以通过 /proc/<pid>/task/<tid>/ 获取线程相关信息，但 macos 不支持该操作
            // macos 需要调用SYS_thread_selfid
            #ifdef __APPLE__
                return static_cast<pid_t>(::syscall(SYS_thread_selfid));
            #else
                return static_cast<pid_t>(::syscall(SYS_gettid));
            #endif
        }

        void afterFork()
        {
            CurrentThread::t_cachedTid = 0;
            CurrentThread::t_threadName = "main";
            CurrentThread::tid();
            // no need to call pthread_atfork(NULL, NULL, &afterFork);
        }

        class ThreadNameInitializer
        {
        public:
            ThreadNameInitializer()
            {
                CurrentThread::t_threadName = "main";
                CurrentThread::tid();
                // fork系统调用之前调用atFork
                pthread_atfork(NULL, NULL, afterFork);
            }
        };

        ThreadNameInitializer init;

        struct ThreadData
        {
            typedef Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;
            std::string name_;
            pid_t *tid_;
            CountDownLatch *latch_;

            ThreadData(ThreadFunc func,
                       const std::string& name,
                       pid_t *tid,
                       CountDownLatch *latch
                       )
                       : func_(std::move(func)),
                       name_(name),
                       tid_(tid),
                       latch_(latch)
            {}

            void runInThread()
            {
                *tid_ = CurrentThread::tid();
                std::cout << *tid_ << std::endl;
                tid_ = NULL;
                latch_->countDown();
                latch_ = NULL;

                CurrentThread::t_threadName = name_.empty() ? "zhengqiThread" : name_.c_str();
                // WARN: macos中没有该系统调用，因此不设置该task_struct的名称
                // ::prctl(PR_SET_NAME, CurrentThread::t_threadName);
                try
                {
                    func_();
                    CurrentThread::t_threadName = "finished";
                }
                catch (const Exception& ex)
                {
                    CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
                    abort();
                }
                catch (const std::exception& ex)
                {
                    CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    abort();
                }
                catch (...)
                {
                    CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
                    throw; // rethrow
                }
            }
        };

        void* startThread(void *obj)
        {
            ThreadData *data = static_cast<ThreadData*>(obj);
            data->runInThread();
            delete data;
            return NULL;
        }

        void CurrentThread::cacheTid()
        {
            if (t_cachedTid == 0)
            {
                t_cachedTid = gettid();
                t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d", t_cachedTid);
            }
        }

        bool CurrentThread::isMainThread()
        {
            return tid() == ::getpid();
        }

        void CurrentThread::sleepUsec(int64_t usec)
        {

            struct timespec ts = {0, 0};
            ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
            ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
            ::nanosleep(&ts, NULL);
        }

        // 统计Thread类的创建实例数
        AtomicInt32 Thread::numCreated_;

        Thread::Thread(ThreadFunc func, const std::string &name)
            : started_(false),
            joined_(false),
            pthreadId_(0),
            tid_(0),
            func_(std::move(func)),
            name_(name),
            latch_(1)
        {
            setDefaultName();
        }

        Thread::~Thread()
        {
            if (started_ && !joined_)
            {
                // 只设置线程的分离属性，不强制终止线程
                pthread_detach(pthreadId_);
            }
        }

        void Thread::setDefaultName()
        {
            int num = numCreated_.incrementAndGet();
            if (name_.empty())
            {
                char buf[32];
                snprintf(buf, sizeof buf, "Thread%d", num);
                name_ = buf;
            }
        }

        void Thread::start()
        {
            assert(!started_);
            started_ = true;
            ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);

            if (pthread_create(&pthreadId_, NULL, &startThread, data))
            {
                // pthread_create 创建线程失败
                started_ = false;
                delete data;
            }
            else
            {
                // 原线程等待 startThread 设置好线程数据
                latch_.wait();
                assert(tid_ > 0);
            }
        }

        int Thread::join()
        {
            assert(started_);
            assert(!joined_);
            joined_ = true;
            return pthread_join(pthreadId_, NULL);
        }

    } // zhengqi
} // utility