#pragma once

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace ccl {

template<typename T>
class BlockingQueue final {
private:
    const size_t m_capacity;
    std::queue<T> m_queue;
    std::condition_variable m_condition;
    std::mutex m_mutex;

public:
    BlockingQueue(size_t capacity = SIZE_MAX) : m_capacity(capacity ? capacity : SIZE_MAX) {};

    ~BlockingQueue() = default;
    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;

    bool Empty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t Size() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void Push(const T& element) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_queue.size() + 1 > m_capacity) {
                m_condition.wait(lock);
            }
            m_queue.push(element);
        }
        m_condition.notify_one();
    }

    void Push(T&& element) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_queue.size() + 1 > m_capacity) {
                m_condition.wait(lock);
            }
            m_queue.push(std::move(element));
        }
        m_condition.notify_one();
    }

    template<class Rep, class Period>
    std::cv_status Push(const T& element, const std::chrono::duration<Rep, Period>& timeout) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_queue.size() + 1 > m_capacity) {
                if (m_condition.wait_for(lock, timeout) == std::cv_status::timeout) {
                    return std::cv_status::timeout;
                }
            }
            m_queue.push(element);
        }
        m_condition.notify_one();
        return std::cv_status::no_timeout;
    }

    template<class Rep, class Period>
    std::cv_status Push(T&& element, const std::chrono::duration<Rep, Period>& timeout) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_queue.size() + 1 > m_capacity) {
                if (m_condition.wait_for(lock, timeout) == std::cv_status::timeout) {
                    return std::cv_status::timeout;
                }
            }
            m_queue.push(std::move(element));
        }
        m_condition.notify_one();
        return std::cv_status::no_timeout;
    }

    T Pop() {
        T element;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_queue.empty()) {
                m_condition.wait(lock);
            }
            element = std::move(m_queue.front());
            m_queue.pop();
        }
        m_condition.notify_one();
        return element;
    }

    template<class Rep, class Period>
    std::cv_status Pop(const std::chrono::duration<Rep, Period>& timeout, T* element) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_queue.empty()) {
                if (m_condition.wait_for(lock, timeout) == std::cv_status::timeout) {
                    return std::cv_status::timeout;
                }
            }
            if (element != nullptr) {
                *element = std::move(m_queue.front());
            }
            m_queue.pop();
        }
        m_condition.notify_one();
        return std::cv_status::no_timeout;
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty()) {
            m_queue.pop();
        }
    }
};

} // namespace ccl
