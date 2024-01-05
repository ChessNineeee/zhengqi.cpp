#ifndef ZHENGQI_CPP_ASSERT2_H
#define ZHENGQI_CPP_ASSERT2_H

#include <iostream>

#define assert_2(condition, message)                                           \
  (!(condition))                                                               \
      ? (std::cerr << "Assertion failed: (" << #condition << "), "             \
                   << "function " << __FUNCTION__ << ", file " << __FILE__     \
                   << ", line " << __LINE__ << "." << std::endl                \
                   << message << std::endl,                                    \
         abort(), 0)                                                           \
      : 1
#endif