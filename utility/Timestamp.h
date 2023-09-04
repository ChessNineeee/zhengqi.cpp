//
// Created by zhengqi on 2023/9/3.
//

#ifndef CPP_TIMESTAMP_H
#define CPP_TIMESTAMP_H

#include <stdint.h>
#include <algorithm>
#include "copyable.h"
#include "boost_operator.h"
#include "Types.h"

namespace zhengqi {
    namespace utility {

    class Timestamp : public copyable,
        public boost_port_operators::equality_comparable<Timestamp>,
        public boost_port_operators::less_than_comparable<Timestamp>
    {
    public:
        ///
        /// Constucts an invalid Timestamp.
        ///
        Timestamp()
                : microSecondsSinceEpoch_(0)
        {
        }

        ///
        /// Constucts a Timestamp at specific time
        ///
        /// @param microSecondsSinceEpoch
        explicit Timestamp(int64_t microSecondsSinceEpochArg)
                : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
        {
        }

        void swap(Timestamp& that)
        {
            std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
        }

        // default copy/assignment/dtor are Okay

        string toString() const;
        string toFormattedString(bool showMicroseconds = true) const;

        bool valid() const { return microSecondsSinceEpoch_ > 0; }

        // for internal usage.
        int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
        time_t secondsSinceEpoch() const
        { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }

        ///
        /// Get time of now.
        ///
        static Timestamp now();
        static Timestamp invalid()
        {
            return Timestamp();
        }

        static Timestamp fromUnixTime(time_t t)
        {
            return fromUnixTime(t, 0);
        }

        static Timestamp fromUnixTime(time_t t, int microseconds)
        {
            return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
        }

        static const int kMicroSecondsPerSecond = 1000 * 1000;
    private:
        int64_t microSecondsSinceEpoch_;
    };

    inline bool operator<(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator==(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }

    ///
    /// Gets time difference of two timestamps, result in seconds.
    ///
    /// @param high, low
    /// @return (high-low) in seconds
    /// @c double has 52-bit precision, enough for one-microsecond
    /// resolution for next 100 years.
    inline double timeDifference(Timestamp high, Timestamp low)
    {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

    ///
    /// Add @c seconds to given timestamp.
    ///
    /// @return timestamp+seconds as Timestamp
    ///
    inline Timestamp addTime(Timestamp timestamp, double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
    }

    } // zhengqi
} // utility

#endif //CPP_TIMESTAMP_H
