#pragma once

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "ccl/thread_pool.h"
#ifdef CCL_USE_BOOST_ANY
 #include <boost/any.hpp>
#else
 #include "ccl/any.h"
#endif // CCL_USE_BOOST_ANY

namespace ccl {

#ifdef CCL_USE_BOOST_ANY
using any = boost::any;
#endif // CCL_USE_BOOST_ANY

class Actor final {
private:
    const std::function<any(any&&)> m_onReceive;
    std::shared_ptr<ThreadPool> m_pool;

public:
    explicit Actor(std::function<any(any&&)>&& onReceive)
            : m_onReceive(std::move(onReceive)), m_pool(std::make_shared<ThreadPool>(1)) {}

    Actor(const std::shared_ptr<ThreadPool>& pool, std::function<any(any&&)>&& onReceive)
            : m_onReceive(std::move(onReceive)), m_pool(pool) {}

    ~Actor() = default;
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    std::future<any> Send(const any& message) {
        struct Func {
            Actor* actor;
            any msg;
            any operator()() {
                return actor->m_onReceive(std::move(msg));
            }
        };
        auto task = std::make_shared<std::packaged_task<any()>>(Func{this, message});
        m_pool->Dispatch([task]() { (*task)(); });
        return task->get_future();
    }

    void SetShutdownNow(bool shutdownNow) {
        m_pool->SetShutdownNow(shutdownNow);
    }
};

class ActorNameSystem final {
private:
    std::map<std::string, std::shared_ptr<Actor>> m_actors;
    std::mutex m_mutex;

public:
    ActorNameSystem() = default;
    ~ActorNameSystem() = default;
    ActorNameSystem(const ActorNameSystem&) = delete;
    ActorNameSystem& operator=(const ActorNameSystem&) = delete;

    void Register(const std::string& name, const std::shared_ptr<Actor>& actor) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_actors[name] = actor;
    }

    void Unregister(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_actors.erase(name);
    }

    std::shared_ptr<Actor> Lookup(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_actors[name];
    }
};

} // namespace ccl
