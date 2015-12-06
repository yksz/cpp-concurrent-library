#pragma once

#include <cstddef>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace ccl {

template<typename T>
class BlockingQueue final {
public:
    BlockingQueue(size_t capacity = 0) : m_capacity(capacity) {};
    ~BlockingQueue() = default;
    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;

    bool Empty();
    size_t Size();
    void Push(const T& element);
    void Emplace(T&& element);
    T Pop();
    void Pop(T& element);
    void Clear();

private:
    const size_t m_capacity;
    std::queue<T> m_queue;
    std::condition_variable m_condition;
    std::mutex m_mutex;
};

template<typename T>
bool BlockingQueue<T>::Empty() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

template<typename T>
size_t BlockingQueue<T>::Size() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.size();
}

template<typename T>
void BlockingQueue<T>::Push(const T& element) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.size() > m_capacity - 1) {
            m_condition.wait(lock);
        }
        m_queue.push(element);
    }
    m_condition.notify_one();
}

template<typename T>
void BlockingQueue<T>::Emplace(T&& element) {
    std::unique_lock<std::mutex> lock(m_mutex);
    {
        while (m_queue.size() > m_capacity - 1) {
            m_condition.wait(lock);
        }
        m_queue.emplace(std::move(element));
    }
    m_condition.notify_one();
}

template<typename T>
T BlockingQueue<T>::Pop() {
    T element;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty()) {
            m_condition.wait(lock);
        }
        element = m_queue.front();
        m_queue.pop();
    }
    m_condition.notify_one();
    return element;
}

template<typename T>
void BlockingQueue<T>::Pop(T& element) {
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

template<typename T>
void BlockingQueue<T>::Clear() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (!m_queue.empty()) {
        m_queue.pop();
    }
}

} // namespace ccl
