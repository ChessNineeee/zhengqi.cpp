//
// Created by zhengqi on 2023/9/2.
//

#ifndef CPP_SINGLETON_H
#define CPP_SINGLETON_H

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include "noncopyable.h"

namespace zhengqi
{
    namespace utility
    {
        template<typename T>
        struct has_no_destroy
        {
            // 如果类型T有no_destroy成员，则会匹配该模板函数
            template <typename C> static char test(decltype(&C::no_destroy));
            // 否则匹配该模板函数
            template <typename C> static int32_t test(...);
            // 如果类型T有no_destroy成员，则返回true，否则返回false
            const static bool value = sizeof(test<T>(0)) == 1;
        };


        template<typename T>
        class Singleton : noncopyable
        {
        public:
            Singleton() = delete;
            ~Singleton() = delete;

            static T& instance()
            {
                pthread_once(&ponce_, &Singleton::init);
                assert(value_ != NULL);
                return *value_;
            }

        private:
            static void init()
            {
                value_ = new T();
                if (!has_no_destroy<T>::value)
                {
                    ::atexit(destroy);
                }
            }

            static void destroy()
            {
                // 不完全类型的大小在编译期不能确定，sizeof结果为0
                // 因此 T_must_be_complete_type的类型为char[-1]
                // 由于负数不能够作为数组的类型，所以编译器在这里会报错
                // 这么做的目的是避免程序在运行时delete 不完全类型的指针
                // 因为delete 不完全类型的指针是不安全的
                typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
                T_must_be_complete_type dummy; (void) dummy;
                delete value_;
                value_ = NULL;
            }
        private:
            static pthread_once_t ponce_;
            static T* value_;
        };

        template<typename T>
        pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

        template<typename T>
        T* Singleton<T>::value_ = NULL;
    }
}

#endif //CPP_SINGLETON_H
