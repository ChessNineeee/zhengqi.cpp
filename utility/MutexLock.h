//
// Created by zhengqi on 2023/9/1.
//

#ifndef CPP_MUTEXLOCK_H
#define CPP_MUTEXLOCK_H

#include <assert.h>
#include <pthread.h>
#include "noncopyable.h"
#include "CurrentThread.h"

// muduo 中针对 clang 编译器有特殊的属性设置，这里忽略了

#ifdef CHECK_PTHREAD_RETURN_VALUE
#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
noexcept __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE
#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret); \
                        assert(errnum == 0); (void) errnum;                                   \
                    })
#endif
namespace zhengqi
{
    namespace utility
    {
        // Use as data member of a class, eg.
        //
        // class Foo
        // {
        //  public:
        //   int size() const;
        //
        //  private:
        //   mutable MutexLock mutex_;
        //   std::vector<int> data_ GUARDED_BY(mutex_);
        // };
        class MutexLock : noncopyable
        {
        public:
            MutexLock()
                :holder_(0)
            {
                MCHECK(pthread_mutex_init(&mutex_, NULL));
            }

            ~MutexLock()
            {
                assert(holder_ == 0);
                MCHECK(pthread_mutex_destroy(&mutex_));
            }

            bool isLockedByThisThread() const
            {
                return holder_ == CurrentThread::tid();
            }

            void assertLocked() const
            {
                assert(isLockedByThisThread());
            }

            void lock()
            {
                MCHECK(pthread_mutex_lock(&mutex_));
                assignHolder();
            }

            void unlock()
            {
                unassignHolder();
                MCHECK(pthread_mutex_unlock(&mutex_));
            }

            pthread_mutex_t* getPthreadMutex()
            {
                return &mutex_;
            }

        private:
            // 友元函数定义
            friend class Condition;
            // UnassignGuard 与条件变量配合使用

            class UnassignGuard : noncopyable
            {
            public:
                // 构造函数，将该互斥量的持有者线程ID设为0
                explicit UnassignGuard(MutexLock& owner)
                    : owner_(owner)
                {
                    owner_.unassignHolder();
                }

                // 析构函数，将该互斥量的持有者线程ID重新设为当前线程ID
                ~UnassignGuard()
                {
                    owner_.assignHolder();
                }

            private:
                MutexLock& owner_;
            };


            void unassignHolder()
            {
                holder_ = 0;
            }

            void assignHolder()
            {
                holder_ = CurrentThread::tid();
            }

            pthread_mutex_t mutex_;
            pid_t holder_;
        };

        // Use as a stack variable, eg.
        // int Foo::size() const
        // {
        //   MutexLockGuard lock(mutex_);
        //   return data_.size();
        // }
        class MutexLockGuard : noncopyable
        {
        public:
            explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex)
            {
                mutex_.lock();
            }
            ~MutexLockGuard()
            {
                mutex_.unlock();
            }
        private:
            MutexLock& mutex_;
        };

    }
}



#endif //CPP_MUTEXLOCK_H
