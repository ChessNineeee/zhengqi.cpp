#ifndef CPP_TIMEZONE_H
#define CPP_TIMEZONE_H

#include "Types.h"
#include "copyable.h"

#include <memory>
#include <time.h>

namespace zhengqi {
    namespace utility {
        struct DateTime {
            DateTime() {}

            explicit DateTime(const struct tm &);

            DateTime(int _year, int _month, int _day, int _hour, int _minute, int _second)
                    : year(_year), month(_month), day(_day), hour(_hour), minute(_minute),
                      second(_second) {}

            string toIsoString() const;

            int year = 0;
            int month = 0;
            int day = 0;
            int hour = 0;
            int minute = 0;
            int second = 0;
        };

        class TimeZone : copyable {
        public:
            TimeZone() = default;

            TimeZone(int eastOfUtc, const char *tzname);

            static TimeZone UTC();

            static TimeZone China();

            static TimeZone loadZoneFile(const char *zoneFile);

            bool valid() const { return static_cast<bool>(data_); }

            struct DateTime toLocalTime(int64_t secondsSinceEpoch,
                                        int *utcOffset = nullptr) const;

            int64_t fromLocalTime(const struct DateTime &,
                                  bool postTransition = false) const;

            // gmtime(3)
            static struct DateTime toUtcTime(int64_t secondsSinceEpoch);

            // timegm(3)
            static int64_t fromUtcTime(const struct DateTime &);

            struct Data;

        private:
            explicit TimeZone(std::unique_ptr<Data> data);

            std::shared_ptr<Data> data_;

            friend class TimeZoneTestPeer;
        };
    } // namespace utility
} // namespace zhengqi
#endif // !DEBUG
