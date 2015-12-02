#include "ccl/actor.h"
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <gtest/gtest.h>
#include "ccl/any.h"

using namespace ccl;

TEST(Actor, Tell) {
    // setup:
    const std::string sentMessage = "message";
    std::string receivedMessage;
    bool received = false;
    std::condition_variable condition;
    std::mutex mutex;

    // when:
    Actor actor([&](const any& message) {
        if (message.type() == typeid(std::string)) {
            receivedMessage = any_cast<std::string>(message);
        }

        // notify
        {
            std::unique_lock<std::mutex> lock(mutex);
            received = true;
        }
        condition.notify_one();
    });
    actor.Tell(sentMessage);

    // wait
    {
        std::unique_lock<std::mutex> lock(mutex);
        while (!received) {
            condition.wait(lock);
        }
    }

    // then:
    EXPECT_EQ(sentMessage, receivedMessage);
}

TEST(ActorSystem, SendAndBroadcast) {
    // setup:
    std::string receivedMessage1;
    std::string receivedMessage2;
    ActorSystem& system = ActorSystem::GetInstance();

    // when:
    {
        auto actor1 = std::make_shared<Actor>([&](const any& message) {
            if (message.type() == typeid(std::string)) {
                receivedMessage1 += any_cast<std::string>(message);
            }
        });
        auto actor2 = std::make_shared<Actor>([&](const any& message) {
            if (message.type() == typeid(std::string)) {
                receivedMessage2 += any_cast<std::string>(message);
            }
        });
        system.Register("/path/actor1", actor1);
        system.Register("/path/actor2", actor2);
    }
    system.Send("/path/actor1", std::string("foo"));
    system.Send("/path/actor2", std::string("fizz"));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    system.Send("/path/actor1", std::string("bar"));
    system.Send("/path/actor2", std::string("bazz"));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    system.Broadcast(std::string("!"));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // then:
    EXPECT_EQ("foobar!", receivedMessage1);
    EXPECT_EQ("fizzbazz!", receivedMessage2);
}
