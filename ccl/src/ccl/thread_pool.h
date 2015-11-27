#pragma once

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include "ccl/blocking_queue.h"

namespace ccl {

class ThreadPool final {
public:
    explicit ThreadPool(std::size_t numThreads);
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void Dispatch(std::function<void()>&& task);

private:
    const std::function<void()> m_poison;
    std::vector<std::thread> m_threads;
    BlockingQueue<std::function<void()>> m_queue;
};

inline ThreadPool::ThreadPool(std::size_t numThreads) {
    for (std::size_t i = 0; i < numThreads; i++) {
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
    for (std::size_t i = 0, n = m_threads.size(); i < n; i++) {
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
