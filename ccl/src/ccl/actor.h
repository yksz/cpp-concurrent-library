#pragma once

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "ccl/any.h"
#include "ccl/thread_pool.h"

namespace ccl {

class Actor final {
public:
    explicit Actor(std::function<any(const any&)>&& onReceive)
            : m_onReceive(std::move(onReceive)), m_pool(std::make_shared<ThreadPool>(1)) {}
    Actor(const std::shared_ptr<ThreadPool>& pool, std::function<any(const any&)>&& onReceive)
            : m_onReceive(std::move(onReceive)), m_pool(pool) {}
    ~Actor() = default;
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    std::future<any> Send(const any& message);
    void SetShutdownNow(bool shutdownNow) {
        m_pool->SetShutdownNow(shutdownNow);
    }

private:
    const std::function<any(const any&)> m_onReceive;
    std::shared_ptr<ThreadPool> m_pool;
};

inline std::future<any> Actor::Send(const any& message) {
    auto task = std::make_shared<std::packaged_task<any()>>([this, message]() {
        return m_onReceive(message);
    });
    m_pool->Dispatch([task]() { (*task)(); });
    return task->get_future();
}


class ActorSystem final {
public:
    ActorSystem() = default;
    ~ActorSystem() = default;
    ActorSystem(const ActorSystem&) = delete;
    ActorSystem& operator=(const ActorSystem&) = delete;

    void Register(const std::string& address, const std::shared_ptr<Actor>& actor);
    bool Unregister(const std::string& address);
    std::future<any> Send(const std::string& address, const any& message);
    void Broadcast(const any& message);

private:
    std::map<std::string, std::shared_ptr<Actor>> m_actors;
    std::mutex m_mutex;
};

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
