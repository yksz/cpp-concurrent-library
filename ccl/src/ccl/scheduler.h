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
    int64_t executionTime; // unix time [ms]
    int64_t period; // [ms]
    int16_t repeatCount;
};

struct scheduledTaskComparator {
    bool operator()(const scheduledTask& a, const scheduledTask& b) {
        return a.executionTime > b.executionTime;
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
                    auto now = Scheduler::ToUnixTime(system_clock::now());
                    if (schedTask.executionTime <= now) { // fired
                        m_queue.pop();
                        if (schedTask.period > 0 && schedTask.repeatCount != 0) { // repeat
                            schedTask.executionTime += schedTask.period;
                            schedTask.repeatCount--;
                            m_queue.push(schedTask); // reschedule
                        }
                    } else {
                        auto execTime = Scheduler::ToTimePoint<system_clock>(schedTask.executionTime);
                        m_condition.wait_until(lock, execTime);
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

    void Schedule(int64_t startTime, std::function<void()>&& task) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(scheduledTask{task, startTime, 0, 0});
        }
        m_condition.notify_one();
    }

    // Schedules the task for repeated execution.
    // Executes the task forever if repeatCount is less than 0.
    void SchedulePeriodically(int64_t firstTime, int64_t period, int16_t repeatCount,
            std::function<void()>&& task) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(scheduledTask{task, firstTime, period, repeatCount});
        }
        m_condition.notify_one();
    }

    template<typename T>
    static int64_t ToUnixTime(const std::chrono::time_point<T>& tp) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
    }

    template<typename T>
    static std::chrono::time_point<T> ToTimePoint(int64_t unixTime) {
        return std::chrono::time_point<T>(std::chrono::duration<int64_t, std::milli>(unixTime));
    }
};

} // namespace ccl
