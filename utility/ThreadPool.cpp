//
// Created by zhengqi on 2023/9/3.
//

#include "ThreadPool.h"
#include "Exception.h"

using namespace zhengqi::utility;

ThreadPool::ThreadPool(const string &nameArg)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(nameArg),
      maxQueueSize_(0),
      running_(false)
{
}

ThreadPool::~ThreadPool()
{
    if (running_)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        char id[32];
        snprintf(id, sizeof id, "%d", i+1);
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this), name_ + id));
        threads_[i]->start();
    }

    if (numThreads == 0 && threadInitCallback_)
    {
        threadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        MutexLockGuard lock(mutex_);
        running_ = false;
        notEmpty_.notifyAll();
        notFull_.notifyAll();
    }

    for (auto& thr: threads_)
    {
        thr->join();
    }
}

size_t ThreadPool::queueSize() const
{
    MutexLockGuard lockGuard(mutex_);
    return queue_.size();
}

void ThreadPool::run(Task task)
{
    if (threads_.empty())
    {
        task();
    }
    else
    {
        MutexLockGuard lockGuard(mutex_);
        while (isFull() && running_)
        {
            // 如果任务队列已满则等待
            notFull_.wait();
        }
        // 如果发现线程池已被关闭，则返回
        if (!running_) return;
        assert(!isFull());

        queue_.push_back(std::move(task));
        notEmpty_.notify();
    }
}

ThreadPool::Task ThreadPool::take()
{
    MutexLockGuard lockGuard(mutex_);
    while (queue_.empty() && running_)
    {
        notEmpty_.wait();
    }

    Task task;
    if (!queue_.empty())
    {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize_ > 0)
        {
            notFull_.notify();
        }
    }

    return task;
}

bool ThreadPool::isFull() const
{
    // 临界区内调用的函数
    mutex_.assertLocked();
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread()
{
    try
    {
        if (threadInitCallback_)
        {
            threadInitCallback_();
        }
        while (running_)
        {
            Task task(take());
            if (task)
            {
                task();
            }
        }
    }
    catch (const Exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }
}