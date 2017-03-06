#pragma once

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <functional>
#include <thread>
#include <vector>
#include "ccl/blocking_queue.h"

namespace ccl {

class ThreadPool final {
private:
    std::atomic<bool> m_shutdownNow;
    const std::function<void()> m_poison;
    std::vector<std::thread> m_threads;
    BlockingQueue<std::function<void()>> m_queue;

public:
    explicit ThreadPool(size_t nthreads, size_t queueSize = SIZE_MAX)
            : m_shutdownNow(false), m_queue(queueSize) {
        for (size_t i = 0; i < nthreads; i++) {
            auto worker = [this]() {
                while (true) {
                    std::function<void()> task = m_queue.Pop();
                    if (task.target_type() == m_poison.target_type()) {
                        return;
                    }
                    task();
                }
            };
            m_threads.emplace_back(worker);
        }
    }

    ~ThreadPool() {
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

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void Dispatch(std::function<void()>&& task) {
        if (task.target_type() != m_poison.target_type()) {
            m_queue.Push(std::move(task));
        }
    }

    void SetShutdownNow(bool shutdownNow) {
        m_shutdownNow = shutdownNow;
    }
};

} // namespace ccl
