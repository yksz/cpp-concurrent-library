#pragma once

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include "ccl/any.h"
#include "ccl/thread_pool.h"

#ifdef CCL_ACTOR_DEBUG
#include <cstdio>
#define CCL_ACTOR_DEBUG_PRINTF(fmt, ...) do { printf(fmt, ## __VA_ARGS__); } while (0)
#else
#define CCL_ACTOR_DEBUG_PRINTF(fmt, ...)
#endif // CCL_ACTOR_DEBUG

namespace ccl {

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


class ActorSystem final {
private:
    std::map<std::string, std::shared_ptr<Actor>> m_actors;
    std::mutex m_mutex;

public:
    ActorSystem() = default;
    ~ActorSystem() = default;
    ActorSystem(const ActorSystem&) = delete;
    ActorSystem& operator=(const ActorSystem&) = delete;

    void Register(const std::shared_ptr<Actor>& actor, const std::string& address) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_actors[address] = actor;
    }

    bool Unregister(const std::string& address) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_actors.erase(address) == 1;
    }

    std::future<any> Send(const any& message, const std::string& address
#ifdef CCL_ACTOR_DEBUG
            , const char* from = __FILE__
#endif // CCL_ACTOR_DEBUG
            ) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_actors.find(address) != m_actors.end()) {
            CCL_ACTOR_DEBUG_PRINTF("[ccl/actor] Send from `%s` to `%s`: message=`%s`\n",
                    from, address.c_str(), message.type().name());
            std::shared_ptr<Actor> actor = m_actors[address];
            return actor->Send(message);
        }
        return std::future<any>();
    }

    void Broadcast(const any& message
#ifdef CCL_ACTOR_DEBUG
            , const char* from = __FILE__
#endif // CCL_ACTOR_DEBUG
            ) {
        std::lock_guard<std::mutex> lock(m_mutex);
        CCL_ACTOR_DEBUG_PRINTF("[ccl/actor] Broadcast from `%s`: message=`%s`\n",
                from, message.type().name());
        for (auto& pair : m_actors) {
            pair.second->Send(message);
        }
    }

    void Multicast(const any& message, const std::string& address
#ifdef CCL_ACTOR_DEBUG
            , const char* from = __FILE__
#endif // CCL_ACTOR_DEBUG
            ) {
        std::string pattern = address;
        replace(pattern, "+", "\\w+");
        int last = pattern.size() - 1;
        if (pattern.at(last) == '#') {
            pattern.replace(last, 1, ".+");
        }
        std::regex regex(pattern);
        std::smatch match;

        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& pair : m_actors) {
            if (std::regex_match(pair.first, match, regex)) {
                CCL_ACTOR_DEBUG_PRINTF("[ccl/actor] Multicast from `%s` to `%s`: message=`%s`\n",
                        from, address.c_str(), message.type().name());
                pair.second->Send(message);
            }
        }
    }

private:
    void replace(std::string& str, const std::string& oldstr, const std::string& newstr) {
        std::string::size_type pos = 0;
        while (pos = str.find(oldstr, pos), pos != std::string::npos) {
            str.replace(pos, oldstr.size(), newstr);
            pos += newstr.size();
        }
    }
};

} // namespace ccl
