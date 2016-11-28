#pragma once

#include <cstddef>
#include <cstdint>
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
    BlockingQueue(size_t capacity = 0) : m_capacity(capacity ? capacity : SIZE_MAX) {};

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

    void Pop(T& element) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_queue.empty()) {
                m_condition.wait(lock);
            }
            element = std::move(m_queue.front());
            m_queue.pop();
        }
        m_condition.notify_one();
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty()) {
            m_queue.pop();
        }
    }
};

} // namespace ccl
