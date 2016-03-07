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

using namespace ccl;

TEST(Actor, Send) {
    // setup:
    const std::string sentMessage = "message";
    std::string receivedMessage;

    // when:
    {
        Actor actor([&](const any& message) {
            if (message.type() == typeid(std::string)) {
                receivedMessage = any_cast<std::string>(message);
            }
            return 0;
        });
        actor.Send(sentMessage);
    }

    // then:
    EXPECT_EQ(sentMessage, receivedMessage);
}

TEST(Actor, ShutdownNow) {
    // setup:
    const int sendCount = 10000;
    int sum = 0;

    // when:
    {
        Actor actor([&](const any& message) {
            if (message.type() == typeid(int)) {
                sum += any_cast<int>(message);
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

TEST(ActorSystem, SendAndBroadcast) {
    // setup:
    std::string receivedMessage1;
    std::string receivedMessage2;

    // when:
    ActorSystem system;
    {
        auto actor1 = std::make_shared<Actor>([&](const any& message) {
            if (message.type() == typeid(std::string)) {
                receivedMessage1 += any_cast<std::string>(message);
            }
            return 0;
        });
        auto actor2 = std::make_shared<Actor>([&](const any& message) {
            if (message.type() == typeid(std::string)) {
                receivedMessage2 += any_cast<std::string>(message);
            }
            return 0;
        });
        system.Register(actor1, "/path/actor1");
        system.Register(actor2, "/path/actor2");
    }
    system.Send(std::string("foo"), "/path/actor1");
    system.Send(std::string("fizz"), "/path/actor2");
    system.Send(std::string("bar"), "/path/actor1");
    system.Send(std::string("bazz"), "/path/actor2");
    system.Broadcast(std::string("!"));
    util::await();

    // then:
    EXPECT_EQ("foobar!", receivedMessage1);
    EXPECT_EQ("fizzbazz!", receivedMessage2);
}

TEST(ActorSystem, Unregister) {
    // setup:
    std::atomic<int> count(0);

    // when:
    ActorSystem system;
    {
        auto actor1 = std::make_shared<Actor>([&](const any& message) {
            count++;
            return 0;
        });
        auto actor2 = std::make_shared<Actor>([&](const any& message) {
            count++;
            return 0;
        });
        system.Register(actor1, "/path/actor1");
        system.Register(actor2, "/path/actor2");
    }
    system.Send(0, "/path/actor1");
    system.Send(0, "/path/actor2");
    util::await();
    system.Unregister("/path/actor2");
    system.Broadcast(0);
    util::await();

    // then:
    EXPECT_EQ(3, count);
}
