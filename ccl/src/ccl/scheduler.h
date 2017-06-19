#pragma once

#include <cstdint>
#include <ctime>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace ccl {

struct scheduledTask {
    std::function<void()> task;
    int64_t execTime; // Unix time [ms]
    int64_t period; // [ms]
    int16_t repeatCount;
};

struct scheduledTaskComparator {
    bool operator()(const scheduledTask& a, const scheduledTask& b) {
        return a.execTime > b.execTime;
    }
};

class Scheduler final {
private:
    bool m_stopped;
    std::unique_ptr<std::thread> m_thread;
    std::priority_queue<scheduledTask, std::vector<scheduledTask>, scheduledTaskComparator> m_queue;
    std::condition_variable m_condition;
    std::mutex m_mutex;

public:
    Scheduler() : m_stopped(false) {
        using namespace std::chrono;

        auto worker = [this]() {
            while (true) {
                scheduledTask schedTask;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    while (!m_stopped && m_queue.empty()) {
                        m_condition.wait(lock);
                    }
                    if (m_stopped && m_queue.empty()) {
                        return;
                    }
                    schedTask = m_queue.top();
                    auto now = Scheduler::toUnixTime(system_clock::now());
                    if (schedTask.execTime <= now) { // fired
                        m_queue.pop();
                        if (schedTask.period > 0 && schedTask.repeatCount != 0) { // repeat
                            schedTask.execTime += schedTask.period;
                            schedTask.repeatCount--;
                            m_queue.push(schedTask); // reschedule
                        }
                    } else {
                        auto execTp = Scheduler::toTimePoint<system_clock>(schedTask.execTime);
                        m_condition.wait_until(lock, execTp);
                        continue;
                    }
                }
                schedTask.task();
            }
        };
        m_thread = std::unique_ptr<std::thread>(new std::thread(std::move(worker)));
    }

    ~Scheduler() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopped = true;
            // clear
            while (!m_queue.empty()) {
                m_queue.pop();
            }
        }
        m_condition.notify_one();
        m_thread->join();
    }

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    template<class Clock>
    void Schedule(const std::chrono::time_point<Clock>& startTime, std::function<void()>&& task) {
        int64_t startUnixTime = Scheduler::toUnixTime(startTime);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(scheduledTask{task, startUnixTime, 0, 0});
        }
        m_condition.notify_one();
    }


    // Schedules the task for repeated execution.
    // Executes the task forever if repeatCount is less than 0.
    template<class Clock, class Rep, class Period>
    void Schedule(const std::chrono::time_point<Clock>& firstTime,
            const std::chrono::duration<Rep, Period>& period, int16_t repeatCount, std::function<void()>&& task) {
        int64_t firstUnixTime = Scheduler::toUnixTime(firstTime);
        int64_t periodMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(period).count();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(scheduledTask{task, firstUnixTime, periodMilliseconds, repeatCount});
        }
        m_condition.notify_one();
    }

private:
    template<typename Clock>
    static int64_t toUnixTime(const std::chrono::time_point<Clock>& tp) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
    }

    template<typename Clock>
    static std::chrono::time_point<Clock> toTimePoint(int64_t unixTime) {
        return std::chrono::time_point<Clock>(std::chrono::duration<int64_t, std::milli>(unixTime));
    }
};

} // namespace ccl
