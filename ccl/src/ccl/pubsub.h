#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <vector>
#include "ccl/actor.h"
#include "ccl/any.h"

namespace ccl {

class PubSub final {
private:
    std::map<std::string, std::vector<std::shared_ptr<Actor>>> m_actors;
    std::mutex m_mutex;

public:
    PubSub() = default;
    ~PubSub() = default;
    PubSub(const PubSub&) = delete;
    PubSub& operator=(const PubSub&) = delete;

    void Subscribe(const std::string& topic, const std::shared_ptr<Actor>& actor) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_actors[topic].push_back(actor);
    }

    void Subscribe(const std::string& topic, const std::vector<std::shared_ptr<Actor>>& actors) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& actor : actors) {
            m_actors[topic].push_back(actor);
        }
    }

    void Unsubscribe(const std::string& topic, const std::shared_ptr<Actor>& actor) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& vec = m_actors[topic];
        for (auto it = vec.begin(); it != vec.end(); it++) {
            if (*it == actor) {
                vec.erase(it);
                return;
            }
        }
    }

    void Publish(const std::string& topic, const any& message) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& actor : m_actors[topic]) {
            actor->Send(message);
        }
    }

    void Broadcast(const any& message) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& pair : m_actors) {
            for (auto& actor : pair.second) {
                actor->Send(message);
            }
        }
    }

    void Multicast(const std::string& topic, const any& message) {
        std::string pattern = topic;
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
                for (auto& actor : pair.second) {
                    actor->Send(message);
                }
            }
        }
    }

    std::vector<std::string> GetTopics() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::string> vec;
        for (auto& pair : m_actors) {
            vec.push_back(pair.first);
        }
        return vec;
    }

    std::vector<std::shared_ptr<Actor>> GetSubscribers(const std::string& topic) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_actors[topic];
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
