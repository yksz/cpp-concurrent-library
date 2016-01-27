#pragma once

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

struct ScheduledTask {
    std::function<void()> task;
    long executionTime; // unix time [ms]
    long period; // [ms]
};

struct ScheduledTaskComparator {
    bool operator()(const ScheduledTask& a, const ScheduledTask& b) {
        return a.executionTime > b.executionTime;
    }
};

class Scheduler final {
public:
    Scheduler();
    ~Scheduler();
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    template<typename T>
    static long ToUnixTime(const std::chrono::time_point<T>& tp) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
    }
    template<typename T>
    static std::chrono::time_point<T> ToTimePoint(long unixTime) {
        return std::chrono::time_point<T>(std::chrono::duration<long, std::milli>(unixTime));
    }

    void Schedule(long startTime, std::function<void()>&& task);
    void Schedule(long firstTime, long period, std::function<void()>&& task);

private:
    bool m_stopped;
    std::unique_ptr<std::thread> m_thread;
    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, ScheduledTaskComparator> m_queue;
    std::condition_variable m_condition;
    std::mutex m_mutex;
};

inline Scheduler::Scheduler()
        : m_stopped(false) {
    using namespace std::chrono;

    auto worker = [this]() {
        while (true) {
            ScheduledTask schedTask;
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
                    if (schedTask.period > 0) { // repeat
                        schedTask.executionTime += schedTask.period;
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

inline Scheduler::~Scheduler() {
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

inline void Scheduler::Schedule(long startTime, std::function<void()>&& task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace((ScheduledTask) {task, startTime, 0});
    }
    m_condition.notify_one();
}

inline void Scheduler::Schedule(long firstTime, long period, std::function<void()>&& task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace((ScheduledTask) {task, firstTime, period});
    }
    m_condition.notify_one();
}

} // namespace ccl
