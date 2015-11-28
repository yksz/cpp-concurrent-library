#pragma once

#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
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
            std::string message;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (!m_stopped && m_mailbox.empty()) {
                    m_condition.wait(lock);
                }
                if (m_stopped && m_mailbox.empty()) {
                    return;
                }
                message = std::move(m_mailbox.front());
                m_mailbox.pop();
            }
            m_onReceive(message);
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
    m_thread = nullptr;
}

inline void Actor::Tell(const std::string& message) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_mailbox.push(message);
    }
    m_condition.notify_one();
}


class ActorSystem final {
public:
    static ActorSystem& GetInstance();
    void Register(const std::string& address, const std::shared_ptr<Actor>& actor);
    void Send(const std::string& address, const std::string& message);
    void Broadcast(const std::string& message);

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

inline void ActorSystem::Send(const std::string& address, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_actors.find(address) != m_actors.end()) {
        std::shared_ptr<Actor> actor = m_actors[address];
        actor->Tell(message);
    }
}

inline void ActorSystem::Broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& pair : m_actors) {
        pair.second->Tell(message);
    }
}

} // namespace ccl
