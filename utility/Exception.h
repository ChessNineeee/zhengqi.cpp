//
// Created by zhengqi on 2023/9/3.
//

#ifndef CPP_EXCEPTION_H
#define CPP_EXCEPTION_H

#include <exception>
#include <string>

namespace zhengqi {
    namespace utility {

        class Exception : std::exception {
        public:
            Exception(std::string what);
            ~Exception() noexcept override = default;

            // 在这里允许拷贝构造和拷贝赋值运算

            const char *what() const noexcept override
            {
                return message_.c_str();
            }

            const char *stackTrace() const noexcept
            {
                return stack_.c_str();
            }
        private:
            std::string message_;
            std::string stack_;
        };

    } // zhengqi
} // utility

#endif //CPP_EXCEPTION_H
