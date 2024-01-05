#ifndef CPP_MINUTEHOURCOUNTER_H
#define CPP_MINUTEHOURCOUNTER_H

#include <ctime>
#include <queue>

// A queue with a maximum number of slots, where old data "falls off" the end.
class ConveyorQueue {
  std::queue<int> q;
  int max_items_;
  int total_sum_; // sum of all items in q.

public:
  explicit ConveyorQueue(int max_items)
      : max_items_(max_items), total_sum_(0) {}
  // Increment the value at the back of the queue.
  void AddToBack(int count) {
    if (q.empty())
      Shift(1); // Make sure q has at least 1 item.
    q.back() += count;
    total_sum_ += count;
  }

  // Each value in the queue is shifted forward by 'num_shifted'.
  // New items are initialized to 0.
  // Oldest items will be removed so there are <= max_items.
  void Shift(int num_shifted) {
    // In case too many items shifted, just clear the queue.
    if (num_shifted >= max_items_) {
      q = std::queue<int>();
      total_sum_ = 0;
      return;
    }

    // Push all the needed zeros.
    while (num_shifted > 0) {
      q.push(0);
      num_shifted--;
    }

    // Let all the excess items fall off.
    while (static_cast<int>(q.size()) > max_items_) {
      total_sum_ -= q.front();
      q.pop();
    }
  }

  // Return the total value of all items currently in the queue.
  int TotalSum() const { return total_sum_; }
};

// A class that keeps counts for the past N buckets of time.
class TrailingBucketCounter {
  ConveyorQueue buckets_;
  const int secs_per_bucket_;
  time_t last_update_time_; // the last time Update() was called

  // Calculate how many buckets of time have passed and Shift() accordingly.
  void Update(time_t now) {
    int current_bucket = static_cast<int>(now) / secs_per_bucket_;
    int last_update_bucket =
        static_cast<int>(last_update_time_) / secs_per_bucket_;

    buckets_.Shift(current_bucket - last_update_bucket);
    last_update_time_ = now;
  }

public:
  // Example: TrailingBucketCounter(30, 60) tracks the last 30 minute-buckets of
  // time.
  TrailingBucketCounter(int num_buckets, int secs_per_bucket)
      : buckets_(num_buckets), secs_per_bucket_(secs_per_bucket) {}
  void Add(int count, time_t now) {
    Update(now);
    buckets_.AddToBack(count);
  }
  // Return the total count over the last num_buckets worth of the time
  int TrailingCount(time_t now) {
    Update(now);
    return buckets_.TotalSum();
  }
};

// Track the cumulative counts over the past minute and over the past hour.
// Useful, for example, to track recent bandwidth usage.
class MinuteHourCounter {
  TrailingBucketCounter minute_counts;
  TrailingBucketCounter hour_counts;

public:
  MinuteHourCounter()
      : minute_counts(/* num _buckets = */ 60, /* secs_per_bucket = */ 1),
        hour_counts(/* num_buckets = */ 60, /* secs_per_bucket = */ 60) {}

  // Add a new data point (count >= 0)
  // For the next minute, MinuteCount will be larger by +count
  // For the next hour, HourCount wil be larger by +count.
  void Add(int count) {
    const time_t now = time(nullptr);

    minute_counts.Add(count, now);
    hour_counts.Add(count, now);
  }
  // Return the count over the past 60 seconds.
  int MinuteCount() {
    time_t now = time(nullptr);
    return minute_counts.TrailingCount(now);
  }
  // Return the count over the past 3600 seconds.
  int HourCount() {
    time_t now = time(nullptr);
    return hour_counts.TrailingCount(now);
  }
};
#endif