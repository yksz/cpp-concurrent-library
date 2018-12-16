#pragma once

#include <condition_variable>
#include <mutex>

namespace ccl {

template<typename T>
class Channel final {
private:
    T m_message;
    bool m_recvReady;
    bool m_sendReady;
    std::mutex m_mutex;
    std::condition_variable m_recvCondition;
    std::condition_variable m_sendCondition;

public:
    Channel() : m_recvReady(false), m_sendReady(false) {}

    ~Channel() = default;
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    T Receive() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_sendReady) {
            m_sendCondition.wait(lock);
        }
        m_recvReady = true;
        m_sendReady = false;
        m_recvCondition.notify_all();
        return std::move(m_message);
    }

    void Send(const T& message) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_message = message;
        m_recvReady = false;
        m_sendReady = true;
        m_sendCondition.notify_all();
        while (!m_recvReady) {
            m_recvCondition.wait(lock);
        }
    }

    void operator>>(T& message) {
        message = Receive();
    }

    void operator<<(const T& message) {
        Send(message);
    }
};

} // namespace ccl
