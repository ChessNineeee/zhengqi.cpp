//
// Created by zhengqi on 2023/9/3.
//

#ifndef CPP_COPYABLE_H
#define CPP_COPYABLE_H

namespace zhengqi
{
    namespace utility
    {
        /// A tag class emphasises the objects are copyable.
        /// The empty base class optimization applies.
        /// Any derived class of copyable should be a value type.
        class copyable
        {
        protected:
            copyable() = default;
            ~copyable() = default;
        };
    }
}
#endif //CPP_COPYABLE_H
