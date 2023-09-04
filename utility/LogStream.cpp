//
// Created by 70903 on 2023/9/4.
//

#include "LogStream.h"
#include <algorithm>
#include <limits>
#include <type_traits>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

namespace zhengqi {
    namespace utility {
        const char digits[] = "9876543210123456789";
        const char* zero = digits + 9;
        static_assert(sizeof(digits) == 20, "wrong number of digits");

        const char digitsHex[] = "0123456789ABCDEF";
        static_assert(sizeof digitsHex == 17, "wrong number of digitsHex");

        // Efficient Integer to String Conversions, by Matthew Wilson.
        template<typename T>
        size_t convert(char buf[], T value)
        {
            T i = value;
            char* p = buf;

            do
            {
                int lsd = static_cast<int>(i % 10);
                i /= 10;
                *p++ = zero[lsd];
            } while (i != 0);

            if (value < 0)
            {
                *p++ = '-';
            }
            *p = '\0';
            std::reverse(buf, p);

            return p - buf;
        }

        size_t convertHex(char buf[], uintptr_t value)
        {
            uintptr_t i = value;
            char* p = buf;

            do
            {
                int lsd = static_cast<int>(i % 16);
                i /= 16;
                *p++ = digitsHex[lsd];
            } while (i != 0);

            *p = '\0';
            std::reverse(buf, p);

            return p - buf;
        }

        template class FixedBuffer<kSmallBuffer>;
        template class FixedBuffer<kLargeBuffer>;
        /*
         Format a number with 5 characters, including SI units.
         [0,     999]
         [1.00k, 999k]
         [1.00M, 999M]
         [1.00G, 999G]
         [1.00T, 999T]
         [1.00P, 999P]
         [1.00E, inf)
        */
        std::string formatSI(int64_t s)
        {
            double n = static_cast<double>(s);
            char buf[64];
            if (s < 1000)
                snprintf(buf, sizeof(buf), "%d", s);
            else if (s < 9995)
                snprintf(buf, sizeof(buf), "%.2fk", n/1e3);
            else if (s < 99950)
                snprintf(buf, sizeof(buf), "%.1fk", n/1e3);
            else if (s < 999500)
                snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
            else if (s < 9995000)
                snprintf(buf, sizeof(buf), "%.2fM", n/1e6);
            else if (s < 99950000)
                snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
            else if (s < 999500000)
                snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
            else if (s < 9995000000)
                snprintf(buf, sizeof(buf), "%.2fG", n/1e9);
            else if (s < 99950000000)
                snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
            else if (s < 999500000000)
                snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
            else if (s < 9995000000000)
                snprintf(buf, sizeof(buf), "%.2fT", n/1e12);
            else if (s < 99950000000000)
                snprintf(buf, sizeof(buf), "%.1fT", n/1e12);
            else if (s < 999500000000000)
                snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
            else if (s < 9995000000000000)
                snprintf(buf, sizeof(buf), "%.2fP", n/1e15);
            else if (s < 99950000000000000)
                snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
            else if (s < 999500000000000000)
                snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
            else
                snprintf(buf, sizeof(buf), "%.2fE", n/1e18);
            return buf;
        }

        /*
         [0, 1023]
         [1.00Ki, 9.99Ki]
         [10.0Ki, 99.9Ki]
         [ 100Ki, 1023Ki]
         [1.00Mi, 9.99Mi]
        */
        std::string formatIEC(int64_t s)
        {
            double n = static_cast<double>(s);
            char buf[64];
            const double Ki = 1024.0;
            const double Mi = Ki * 1024.0;
            const double Gi = Mi * 1024.0;
            const double Ti = Gi * 1024.0;
            const double Pi = Ti * 1024.0;
            const double Ei = Pi * 1024.0;

            if (n < Ki)
                snprintf(buf, sizeof buf, "%d", s);
            else if (n < Ki*9.995)
                snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
            else if (n < Ki*99.95)
                snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
            else if (n < Ki*1023.5)
                snprintf(buf, sizeof buf, "%.0fKi", n / Ki);

            else if (n < Mi*9.995)
                snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
            else if (n < Mi*99.95)
                snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
            else if (n < Mi*1023.5)
                snprintf(buf, sizeof buf, "%.0fMi", n / Mi);

            else if (n < Gi*9.995)
                snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
            else if (n < Gi*99.95)
                snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
            else if (n < Gi*1023.5)
                snprintf(buf, sizeof buf, "%.0fGi", n / Gi);

            else if (n < Ti*9.995)
                snprintf(buf, sizeof buf, "%.2fTi", n / Ti);
            else if (n < Ti*99.95)
                snprintf(buf, sizeof buf, "%.1fTi", n / Ti);
            else if (n < Ti*1023.5)
                snprintf(buf, sizeof buf, "%.0fTi", n / Ti);

            else if (n < Pi*9.995)
                snprintf(buf, sizeof buf, "%.2fPi", n / Pi);
            else if (n < Pi*99.95)
                snprintf(buf, sizeof buf, "%.1fPi", n / Pi);
            else if (n < Pi*1023.5)
                snprintf(buf, sizeof buf, "%.0fPi", n / Pi);

            else if (n < Ei*9.995)
                snprintf(buf, sizeof buf, "%.2fEi", n / Ei );
            else
                snprintf(buf, sizeof buf, "%.1fEi", n / Ei );
            return buf;
        }
    } // zhengqi
} // utility