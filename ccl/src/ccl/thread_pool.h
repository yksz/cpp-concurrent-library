#pragma once

#include <cstddef>
#include <atomic>
#include <functional>
#include <thread>
#include <vector>
#include "ccl/blocking_queue.h"

namespace ccl {

class ThreadPool final {
public:
    explicit ThreadPool(size_t nThreads, size_t queueSize = 0);
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void Dispatch(std::function<void()>&& task);
    void SetShutdownNow(bool shutdownNow) {
        m_shutdownNow = shutdownNow;
    }

private:
    std::atomic<bool> m_shutdownNow;
    const std::function<void()> m_poison;
    std::vector<std::thread> m_threads;
    BlockingQueue<std::function<void()>> m_queue;
};

inline ThreadPool::ThreadPool(size_t nThreads, size_t queueSize)
        : m_shutdownNow(false), m_queue(queueSize) {
    for (size_t i = 0; i < nThreads; i++) {
        auto worker = [this]() {
            while (true) {
                std::function<void()> task;
                m_queue.Pop(task);
                if (task.target_type() == m_poison.target_type()) {
                    return;
                }
                task();
            }
        };
        m_threads.emplace_back(worker);
    }
}

inline ThreadPool::~ThreadPool() {
    if (m_shutdownNow) {
        m_queue.Clear();
    }
    for (size_t i = 0, n = m_threads.size(); i < n; i++) {
        m_queue.Push(m_poison);
    }
    for (std::thread& th : m_threads) {
        th.join();
    }
}

inline void ThreadPool::Dispatch(std::function<void()>&& task) {
    if (task.target_type() != m_poison.target_type()) {
        m_queue.Emplace(std::move(task));
    }
}

} // namespace ccl
