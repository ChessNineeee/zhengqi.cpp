//
// Created by 70903 on 2023/9/7.
//
#include "utility/Date.h"
#include <stdio.h>
#include <time.h>
#include <assert.h>

const int kMonthsOfYear = 12;

int isLeapYear(int year) {
    if (year % 400 == 0)
        return 1;
    else if (year % 100 == 0)
        return 0;
    else if (year % 4 == 0)
        return 1;
    else
        return 0;
}

int daysOfMonth(int year, int month) {
    static int days[2][kMonthsOfYear + 1] = {
            {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
            {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    return days[isLeapYear(year)][month];
}

void passByConstReference(const zhengqi::utility::Date &x) {
    printf("%s\n", x.toIsoString().c_str());
}

void passByValue(zhengqi::utility::Date x) {
    printf("%s\n", x.toIsoString().c_str());
}

int main() {
    time_t now(NULL);
    struct tm t1 = *gmtime(&now);
    struct tm t2 = *localtime(&now);
    zhengqi::utility::Date someDay(2023, 9, 7);
    printf("%s\n", someDay.toIsoString().c_str());
    passByValue(someDay);
    passByConstReference(someDay);
    zhengqi::utility::Date todayUtc(t1);
    printf("%s\n", todayUtc.toIsoString().c_str());
    zhengqi::utility::Date todayLocal(t2);
    printf("%s\n", todayLocal.toIsoString().c_str());

    int julianDayNumber = 2415021;
    int weekDay = 1;

    for (int year = 1900; year < 2500; ++year) {
        assert(zhengqi::utility::Date(year, 3, 1).julianDayNumber() -
               zhengqi::utility::Date(year, 2, 29).julianDayNumber()
               == isLeapYear(year));
        for (int month = 1; month <= kMonthsOfYear; ++month) {
            for (int day = 1; day <= daysOfMonth(year, month); ++day) {
                zhengqi::utility::Date d(year, month, day);
                assert(year == d.year());
                assert(month == d.month());
                assert(day == d.day());
                assert(weekDay == d.weekDay());
                assert(julianDayNumber == d.julianDayNumber());
                zhengqi::utility::Date d2(julianDayNumber);
                assert(year == d2.year());
                assert(month == d2.month());
                assert(day == d2.day());
                assert(weekDay == d2.weekDay());
                assert(julianDayNumber == d2.julianDayNumber());
                ++julianDayNumber;
                weekDay = (weekDay + 1) % zhengqi::utility::Date::kDaysPerWeek;
            }
        }
    }
    printf("All passed.\n");
}