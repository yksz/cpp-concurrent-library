#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace ccl {

class Actor final {
public:
    explicit Actor(std::function<void(const std::string&)>&& onReceive);
    ~Actor();
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    void Tell(const std::string& message);

private:
    std::function<void(const std::string&)> m_onReceive;
    bool m_stopped;
    std::thread* m_thread;
    std::queue<std::string> m_mailbox;
    std::condition_variable m_condition;
    std::mutex m_mutex;
};

inline Actor::Actor(std::function<void(const std::string&)>&& onReceive)
        : m_onReceive(std::move(onReceive)), m_stopped(false) {
    auto worker = [this]() {
        while (true) {
            std::string mail;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (!m_stopped && m_mailbox.empty()) {
                    m_condition.wait(lock);
                }
                if (m_stopped && m_mailbox.empty()) {
                    return;
                }
                mail = std::move(m_mailbox.front());
                m_mailbox.pop();
            }
            m_onReceive(mail);
        }
    };
    m_thread = new std::thread(worker);
}

inline Actor::~Actor() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stopped = true;
    }
    m_condition.notify_one();
    m_thread->join();
    delete m_thread;
}

inline void Actor::Tell(const std::string& message) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_mailbox.push(message);
    }
    m_condition.notify_one();
}

} // namespace ccl
