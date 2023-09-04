//
// Created by zhengqi on 2023/9/1.
//

#ifndef CPP_CURRENTTHREAD_H
#define CPP_CURRENTTHREAD_H
#include "utility/Types.h"

namespace zhengqi
{
    namespace utility
    {
        namespace CurrentThread
        {
            extern __thread int t_cachedTid;
            extern __thread char t_tidString[32];
            extern __thread int t_tidStringLength;
            extern __thread const char* t_threadName;
            void cacheTid();

            inline int tid()
            {
                if (__builtin_expect(t_cachedTid == 0, 0))
                {
                    cacheTid();
                }
                return t_cachedTid;
            }

            inline const char* tidString() // for logging
            {
                return t_tidString;
            }

            inline int tidStringLength() // for logging
            {
                return t_tidStringLength;
            }

            inline const char* name()
            {
                return t_threadName;
            }

            bool isMainThread();

            void sleepUsec(int64_t usec);  // for testing

            string stackTrace(bool demangle);
        }
    }
}

#endif //CPP_CURRENTTHREAD_H
