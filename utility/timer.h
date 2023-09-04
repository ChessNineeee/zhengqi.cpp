//
// Created by zhengqi on 2023/7/27.
//

#ifndef CPP_TIMER_H
#define CPP_TIMER_H

#include <thread>
#include <atomic>
#include <functional>

namespace zhengqi {
    namespace utility {
        class Timer
        {
        public:
            Timer();
            Timer(int repeat);
            ~Timer();

            template<class F, class... Args>
            void start(int millisecond, F && func, Args&&... args);

            void stop();

        private:
            std::thread m_thread;
            std::atomic<bool> m_active;
            std::function<void()> m_func;
            int m_period_ms;
            int m_repeat_times;
        };

        template<class F, class... Args>
        void Timer::start(int millisecond, F&& func, Args&&... args)
        {
            if(m_active.load())
            {
                return;
            }

            m_period_ms = millisecond;
            m_func = std::bind(func, std::ref(args)...);
            m_active.store(true);

            m_thread = std::thread([&](){
                if(m_repeat_times < 0)
                {
                    while(m_active.load())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(m_period_ms));
                        m_func();
                    }
                } else
                {
                    while(m_active.load() && m_repeat_times > 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(m_period_ms));
                        m_func();
                        m_repeat_times -= 1;
                    }
                }
            });
            m_thread.detach();
        }
    }
}
#endif //CPP_TIMER_H
