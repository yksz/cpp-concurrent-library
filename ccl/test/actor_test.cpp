#include "ccl/actor.h"
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <gtest/gtest.h>

TEST(Actor, Tell) {
    // setup:
    const std::string sentMessage = "message";
    std::string receivedMessage;
    bool received = false;
    std::condition_variable condition;
    std::mutex mutex;

    // when:
    ccl::Actor actor([&](const std::string& message) {
        receivedMessage = message;

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
    ccl::ActorSystem& system = ccl::ActorSystem::GetInstance();

    // when:
    {
        auto actor1 = std::make_shared<ccl::Actor>([&](const std::string& message) {
            receivedMessage1 += message;
        });
        auto actor2 = std::make_shared<ccl::Actor>([&](const std::string& message) {
            receivedMessage2 += message;
        });
        system.Register("/path/actor1", actor1);
        system.Register("/path/actor2", actor2);
    }
    system.Send("/path/actor1", "foo");
    system.Send("/path/actor2", "fizz");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    system.Send("/path/actor1", "bar");
    system.Send("/path/actor2", "bazz");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    system.Broadcast("!");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // then:
    EXPECT_EQ(receivedMessage1, "foobar!");
    EXPECT_EQ(receivedMessage2, "fizzbazz!");
}
