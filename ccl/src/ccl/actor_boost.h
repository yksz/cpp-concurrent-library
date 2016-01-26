#pragma once

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <boost/any.hpp>
#include "ccl/thread_pool.h"

namespace ccl {

class Actor final {
public:
    explicit Actor(std::function<boost::any(const boost::any&)>&& onReceive)
            : m_onReceive(std::move(onReceive)), m_pool(std::make_shared<ThreadPool>(1)) {}
    Actor(const std::shared_ptr<ThreadPool>& pool, std::function<boost::any(const boost::any&)>&& onReceive)
            : m_onReceive(std::move(onReceive)), m_pool(pool) {}
    ~Actor() = default;
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    std::future<boost::any> Send(const boost::any& message);
    void SetShutdownNow(bool shutdownNow) {
        m_pool->SetShutdownNow(shutdownNow);
    }

private:
    const std::function<boost::any(const boost::any&)> m_onReceive;
    std::shared_ptr<ThreadPool> m_pool;
};

inline std::future<boost::any> Actor::Send(const boost::any& message) {
    auto task = std::make_shared<std::packaged_task<boost::any()>>([this, message]() {
        return m_onReceive(message);
    });
    m_pool->Dispatch([task]() { (*task)(); });
    return task->get_future();
}


class ActorSystem final {
public:
    static ActorSystem& GetInstance();
    void Register(const std::string& address, const std::shared_ptr<Actor>& actor);
    bool Unregister(const std::string& address);
    std::future<boost::any> Send(const std::string& address, const boost::any& message);
    void Broadcast(const boost::any& message);

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

inline std::future<boost::any> ActorSystem::Send(const std::string& address, const boost::any& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_actors.find(address) != m_actors.end()) {
        std::shared_ptr<Actor> actor = m_actors[address];
        return actor->Send(message);
    }
    return std::future<boost::any>();
}

inline void ActorSystem::Broadcast(const boost::any& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& pair : m_actors) {
        pair.second->Send(message);
    }
}

} // namespace ccl
