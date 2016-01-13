#pragma once

#include <condition_variable>
#include <mutex>

namespace ccl {

class CountdownLatch final {
public:
    CountdownLatch(unsigned int count = 1) : m_count(count) {}
    ~CountdownLatch() = default;
    CountdownLatch(const CountdownLatch&) = delete;
    CountdownLatch& operator=(const CountdownLatch&) = delete;

    template<class Rep, class Period>
    void Await(const std::chrono::duration<Rep, Period>& timeout);
    void Await();
    void CountDown();
    unsigned int GetCount();

private:
    unsigned int m_count;
    std::condition_variable m_condition;
    std::mutex m_mutex;
};

template<class Rep, class Period>
inline void CountdownLatch::Await(const std::chrono::duration<Rep, Period>& timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_count > 0) {
        if (m_condition.wait_for(lock, timeout) == std::cv_status::timeout) {
            return;
        }
    }
}

inline void CountdownLatch::Await() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_count > 0) {
        m_condition.wait(lock);
    }
}

inline void CountdownLatch::CountDown() {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_count <= 0) {
        return;
    }
    m_count--;
    if (m_count == 0) {
        m_condition.notify_one();
    }
}

inline unsigned int CountdownLatch::GetCount() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_count;
}

} // namespace ccl
