//
// Created by zhengqi on 2023/9/3.
//

#ifndef CPP_BOOST_OPERATOR_H
#define CPP_BOOST_OPERATOR_H

namespace zhengqi
{
    namespace utility
    {
        namespace boost_port_operators
        {
            namespace operators_detail
            {
                template <typename T> class empty_base {};
            }
            // boost/operators.hpp 文件中一些简单函数的移植，避免安装整个Boost库
            template <class T, class B = operators_detail::empty_base<T> >
            struct equality_comparable : B
            {
                friend bool operator!=(const T& x, const T& y) { return !static_cast<bool>(x == y); }
            };

            template <class T, class B = operators_detail::empty_base<T> >
            struct less_than_comparable : B
            {
                friend bool operator>(const T& x, const T& y)  { return y < x; }
                friend bool operator<=(const T& x, const T& y) { return !static_cast<bool>(y < x); }
                friend bool operator>=(const T& x, const T& y) { return !static_cast<bool>(x < y); }
            };

        }
    }

}

#endif //CPP_BOOST_OPERATOR_H
