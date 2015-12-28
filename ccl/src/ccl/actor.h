#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include "ccl/any.h"

namespace ccl {

class Actor final {
public:
    explicit Actor(std::function<any(const any&)>&& onReceive);
    ~Actor();
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    std::future<any> Send(const any& message);
    void SetShutdownNow(bool shutdownNow) {
        m_shutdownNow = shutdownNow;
    }

private:
    const std::function<any(const any&)> m_onReceive;
    std::atomic<bool> m_shutdownNow;
    bool m_stopped;
    std::thread* m_thread;
    std::queue<std::packaged_task<any()>> m_mailbox;
    std::condition_variable m_condition;
    std::mutex m_mutex;
};

inline Actor::Actor(std::function<any(const any&)>&& onReceive)
        : m_onReceive(std::move(onReceive)), m_shutdownNow(false), m_stopped(false) {
    auto worker = [this]() {
        while (true) {
            std::packaged_task<any()> task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (!m_stopped && m_mailbox.empty()) {
                    m_condition.wait(lock);
                }
                if (m_stopped && m_mailbox.empty()) {
                    return;
                }
                task = std::move(m_mailbox.front());
                m_mailbox.pop();
            }
            task();
        }
    };
    m_thread = new std::thread(std::move(worker));
}

inline Actor::~Actor() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stopped = true;
        if (m_shutdownNow) {
            // clear
            while (!m_mailbox.empty()) {
                m_mailbox.pop();
            }
        }
    }
    m_condition.notify_one();
    m_thread->join();
    delete m_thread;
    m_thread = nullptr;
}

inline std::future<any> Actor::Send(const any& message) {
    std::future<any> future;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::packaged_task<any()> task([this, message]() {
            return m_onReceive(message);
        });
        future = task.get_future();
        m_mailbox.push(std::move(task));
    }
    m_condition.notify_one();
    return std::move(future);
}


class ActorSystem final {
public:
    static ActorSystem& GetInstance();
    void Register(const std::string& address, const std::shared_ptr<Actor>& actor);
    bool Unregister(const std::string& address);
    std::future<any> Send(const std::string& address, const any& message);
    void Broadcast(const any& message);

private:
    std::map<std::string, std::shared_ptr<Actor>> m_actors;
    std::mutex m_mutex;

    ActorSystem() = default;
    ~ActorSystem() = default;
    ActorSystem(const ActorSystem&) = delete;
    void operator=(const ActorSystem&) = delete;
};

inline ActorSystem& ActorSystem::GetInstance() {
    static ActorSystem instance;
    return instance;
}

inline void ActorSystem::Register(const std::string& address, const std::shared_ptr<Actor>& actor) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_actors[address] = actor;
}

inline bool ActorSystem::Unregister(const std::string& address) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_actors.erase(address) == 1;
}

inline std::future<any> ActorSystem::Send(const std::string& address, const any& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_actors.find(address) != m_actors.end()) {
        std::shared_ptr<Actor> actor = m_actors[address];
        return actor->Send(message);
    }
    return std::future<any>();
}

inline void ActorSystem::Broadcast(const any& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& pair : m_actors) {
        pair.second->Send(message);
    }
}

} // namespace ccl
