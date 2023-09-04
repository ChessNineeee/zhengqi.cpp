//
// Created by zhengqi on 2023/9/1.
//

#ifndef CPP_NONCOPYABLE_H
#define CPP_NONCOPYABLE_H
namespace zhengqi
{
    namespace utility
    {
        class noncopyable
        {
        public:
            noncopyable(const noncopyable&) = delete;
            void operator=(const noncopyable&) = delete;
        protected:
            noncopyable() = default;
            ~noncopyable() = default;
        };
    }
}
#endif //CPP_NONCOPYABLE_H
