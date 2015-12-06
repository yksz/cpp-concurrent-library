#pragma once

#include <ctime>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace {

struct ScheduledTask {
    std::function<void()> task;
    long executionTime; // absolute time
    long period; // [ms]
};

struct ScheduledTaskComparator {
    bool operator()(const ScheduledTask& a, const ScheduledTask& b) {
        return a.executionTime > b.executionTime;
    }
};

} // unnamed namespace

namespace ccl {

class Scheduler final {
public:
    Scheduler();
    ~Scheduler();
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void Schedule(time_t startTime, std::function<void()>&& task);
    void Schedule(time_t firstTime, long period, std::function<void()>&& task);

private:
    bool m_stopped;
    std::thread* m_thread;
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
                time_t now = time(nullptr);
                if (schedTask.executionTime <= now) { // fired
                    m_queue.pop();
                    if (schedTask.period > 0) { // repeat
                        auto tp = system_clock::from_time_t(schedTask.executionTime) + milliseconds(schedTask.period);
                        schedTask.executionTime = system_clock::to_time_t(tp);
                        m_queue.push(schedTask); // reschedule
                    }
                } else {
                    auto tp = system_clock::from_time_t(schedTask.executionTime);
                    m_condition.wait_until(lock, tp);
                    continue;
                }
            }
            schedTask.task();
        }
    };
    m_thread = new std::thread(std::move(worker));
}

inline Scheduler::~Scheduler() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stopped = true;
        // clear
        while (!m_queue.empty()) {
            m_queue.pop();
        }
    }
    m_condition.notify_one();
    m_thread->join();
    delete m_thread;
    m_thread = nullptr;
}

inline void Scheduler::Schedule(time_t startTime, std::function<void()>&& task) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace((ScheduledTask) {task, startTime, 0});
    }
    m_condition.notify_one();
}

inline void Scheduler::Schedule(time_t firstTime, long period, std::function<void()>&& task) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace((ScheduledTask) {task, firstTime, period});
    }
    m_condition.notify_one();
}

} // namespace ccl
