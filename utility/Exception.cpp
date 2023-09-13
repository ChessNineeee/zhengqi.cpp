//
// Created by zhengqi on 2023/9/3.
//

#include "utility/Exception.h"
#include "utility/CurrentThread.h"

namespace zhengqi {
namespace utility {
Exception::Exception(std::string msg)
    : message_(std::move(msg)), stack_(CurrentThread::stackTrace(false)) {}
} // namespace utility
} // namespace zhengqi