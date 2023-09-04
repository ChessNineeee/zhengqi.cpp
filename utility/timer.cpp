//
// Created by zhengqi on 2023/7/27.
//
#include "timer.h"

using namespace zhengqi::utility;

Timer::Timer() : m_active(false), m_period_ms(0), m_repeat_times(-1)
{

}

Timer::Timer(int repeat) : m_active(false), m_period_ms(0), m_repeat_times(repeat)
{

}

Timer::~Timer()
{
    stop();
}

void Timer::stop()
{
    m_active.store(false);
}
