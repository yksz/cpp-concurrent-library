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

TEST(ActorSystem, Unregister) {
    // setup:
    std::atomic<int> count(0);

    // when:
    ActorSystem system;
    {
        auto actor1 = std::make_shared<Actor>([&](any&& msg) {
            count++;
            return 0;
        });
        auto actor2 = std::make_shared<Actor>([&](any&& msg) {
            count++;
            return 0;
        });
        system.Register(actor1, "/path/actor1");
        system.Register(actor2, "/path/actor2");
    }

    // and:
    system.Broadcast(0);
    util::await();

    // then:
    EXPECT_EQ(2, count);

    // when:
    system.Unregister("/path/actor2");
    system.Broadcast(0);
    util::await();

    // then:
    EXPECT_EQ(3, count);
}

TEST(ActorSystem, SendAndBroadcast) {
    // setup:
    std::string recvMsg1;
    std::string recvMsg2;
    CountdownLatch latch(6);

    // when:
    ActorSystem system;
    {
        auto actor1 = std::make_shared<Actor>([&](any&& msg) {
            if (msg.type() == typeid(std::string)) {
                recvMsg1 += any_cast<std::string>(msg);
            }
            latch.CountDown();
            return 0;
        });
        auto actor2 = std::make_shared<Actor>([&](any&& msg) {
            if (msg.type() == typeid(std::string)) {
                recvMsg2 += any_cast<std::string>(msg);
            }
            latch.CountDown();
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
    latch.Await();

    // then:
    EXPECT_EQ("foobar!", recvMsg1);
    EXPECT_EQ("fizzbazz!", recvMsg2);
}

TEST(ActorSystem, Multicast_ForwardMatch) {
    // setup:
    std::atomic<int> count(0);
    bool fail = false;
    CountdownLatch latch(2);

    // when:
    ActorSystem system;
    {
        auto actor1a = std::make_shared<Actor>([&](any&& msg) {
            count++;
            latch.CountDown();
            return 0;
        });
        auto actor2a = std::make_shared<Actor>([&](any&& msg) {
            count++;
            latch.CountDown();
            return 0;
        });
        auto actor2b = std::make_shared<Actor>([&](any&& msg) {
            fail = true;
            return 0;
        });
        system.Register(actor1a, "/a/actor/1");
        system.Register(actor2a, "/a/actor/2");
        system.Register(actor2b, "/b/actor/2");
    }
    system.Multicast(0, "/a/actor/#");
    latch.Await();

    // then:
    EXPECT_EQ(2, count);
    ASSERT_FALSE(fail);
}

TEST(ActorSystem, Multicast_PartialMatch) {
    // setup:
    std::atomic<int> count(0);
    bool fail = false;
    CountdownLatch latch(2);

    // when:
    ActorSystem system;
    {
        auto actor1a = std::make_shared<Actor>([&](any&& msg) {
            fail = true;
            return 0;
        });
        auto actor2a = std::make_shared<Actor>([&](any&& msg) {
            count++;
            latch.CountDown();
            return 0;
        });
        auto actor2b = std::make_shared<Actor>([&](any&& msg) {
            count++;
            latch.CountDown();
            return 0;
        });
        system.Register(actor1a, "/a/actor/1");
        system.Register(actor2a, "/a/actor/2");
        system.Register(actor2b, "/b/actor/2");
    }
    system.Multicast(0, "/+/actor/2");
    latch.Await();

    // then:
    EXPECT_EQ(2, count);
    ASSERT_FALSE(fail);
}
