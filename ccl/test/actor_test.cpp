#include "ccl/actor.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <gtest/gtest.h>
#include "util.h"
#include "ccl/countdown_latch.h"

using namespace ccl;

TEST(Actor, Send) {
    // setup:
    const std::string sendMsg = "message";
    std::string recvMsg;

    // when:
    {
        Actor actor([&](any&& msg) {
            if (msg.type() == typeid(std::string)) {
                recvMsg = any_cast<std::string>(msg);
            }
            return 0;
        });
        actor.Send(sendMsg);
    }

    // then:
    EXPECT_EQ(sendMsg, recvMsg);
}

TEST(Actor, ShutdownNow) {
    // setup:
    const int sendCount = 10000;
    int sum = 0;

    // when:
    {
        Actor actor([&](any&& msg) {
            if (msg.type() == typeid(int)) {
                sum += any_cast<int>(msg);
            }
            return 0;
        });
        for (int i = 0; i < sendCount; i++) {
            actor.Send(1);
        }
        actor.SetShutdownNow(true);
    }

    // then:
    EXPECT_NE(sendCount, sum);
}

TEST(ActorNameSystem, Register) {
    // when:
    ActorNameSystem system;
    {
        auto actor = std::make_shared<Actor>([](any&& msg) {
            return std::string("ok");
        });
        system.Register("actor", actor);
    }

    // then:
    auto actor = system.Lookup("actor");
    EXPECT_NE(nullptr, actor);

    // when:
    auto future = actor->Send(0);

    // then:
    auto response = future.get();
    EXPECT_EQ(typeid(std::string), response.type());
    EXPECT_EQ("ok", any_cast<std::string>(response));
}

TEST(ActorNameSystem, Unregister) {
    // when:
    ActorNameSystem system;
    {
        auto actor = std::make_shared<Actor>([](any&& msg) {
            return 0;
        });
        system.Register("actor", actor);
    }
    system.Unregister("actor");

    // then:
    auto actor = system.Lookup("actor");
    EXPECT_EQ(nullptr, actor);
}
