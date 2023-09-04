//
// Created by 70903 on 2023/9/4.
//

#ifndef CPP_LOGSTREAM_H
#define CPP_LOGSTREAM_H

#include "utility/noncopyable.h"
#include "utility/StringPiece.h"
#include "utility/Types.h"
#include <assert.h>
#include <string.h> // memcpy

namespace zhengqi {
    namespace utility {
        const int kSmallBuffer = 4000; // 4KB
        const int kLargeBuffer = 4000 * 1000; //4MB

        template<int SIZE>
        class FixedBuffer : noncopyable
        {
        public:
        private:
            char data_[SIZE];
            char* cur_;
        };

        class LogStream {

        };

    } // zhengqi
} // utility

#endif //CPP_LOGSTREAM_H
