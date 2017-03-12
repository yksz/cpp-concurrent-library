#pragma once

#include <condition_variable>
#include <mutex>

namespace ccl {

class CountdownLatch final {
private:
    unsigned int m_count;
    std::condition_variable m_condition;
    std::mutex m_mutex;

public:
    CountdownLatch(unsigned int count = 1) : m_count(count) {}

    ~CountdownLatch() = default;
    CountdownLatch(const CountdownLatch&) = delete;
    CountdownLatch& operator=(const CountdownLatch&) = delete;

    void Await() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_count > 0) {
            m_condition.wait(lock);
        }
    }

    template<class Rep, class Period>
    std::cv_status Await(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_count > 0) {
            if (m_condition.wait_for(lock, timeout) == std::cv_status::timeout) {
                return std::cv_status::timeout;
            }
        }
        return std::cv_status::no_timeout;
    }

    void CountDown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_count == 0) {
            return;
        }
        m_count--;
        if (m_count == 0) {
            m_condition.notify_all();
        }
    }

    unsigned int GetCount() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count;
    }
};

} // namespace ccl
