#include "ccl/actor.h"
#include <condition_variable>
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
