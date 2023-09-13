//
// Created by zhengqi on 2023/9/2.
//

#ifndef CPP_COUNTDOWNLATCH_H
#define CPP_COUNTDOWNLATCH_H

#include "utility/Condition.h"
#include "utility/MutexLock.h"
#include "utility/noncopyable.h"

namespace zhengqi {
namespace utility {
class CountDownLatch : noncopyable {
public:
  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};
} // namespace utility
} // namespace zhengqi

#endif // CPP_COUNTDOWNLATCH_H
