#include "ccl/pubsub.h"
#include <gtest/gtest.h>
#include "ccl/countdown_latch.h"

using namespace ccl;

TEST(PubSub, Subscribe) {
    // setup:
    const std::string topic1 = "/topic/1";
    const std::string topic2 = "/topic/2";

    // when:
    auto actor1 = std::make_shared<Actor>([](any&& msg) {
        return 0;
    });
    auto actor2 = std::make_shared<Actor>([](any&& msg) {
        return 0;
    });
    PubSub broker;
    broker.Subscribe(topic1, actor1);
    broker.Subscribe(topic2, {actor1, actor2});

    // then:
    EXPECT_EQ(1, broker.GetSubscribers(topic1).size());
    EXPECT_EQ(2, broker.GetSubscribers(topic2).size());
}

TEST(PubSub, Unsubscribe) {
    // setup:
    const std::string topic = "/topic";

    // when:
    auto actor1 = std::make_shared<Actor>([](any&& msg) {
        return 0;
    });
    auto actor2 = std::make_shared<Actor>([](any&& msg) {
        return 0;
    });
    PubSub broker;
    broker.Subscribe(topic, {actor1, actor2});
    broker.Unsubscribe(topic, actor1);

    // then:
    EXPECT_EQ(1, broker.GetSubscribers(topic).size());
}

TEST(PubSub, Publish) {
    // setup:
    const std::string topic = "/topic";
    std::string recvMsg1;
    std::string recvMsg2;
    CountdownLatch latch(4);

    // when:
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
    PubSub broker;
    broker.Subscribe(topic, {actor1, actor2});
    broker.Publish(topic, std::string("foo"));
    broker.Publish(topic, std::string("bar"));
    broker.Publish("/unknown", std::string("bazz"));
    latch.Await();

    // then:
    EXPECT_EQ("foobar", recvMsg1);
    EXPECT_EQ("foobar", recvMsg2);
}

TEST(PubSub, Broadcast) {
    // setup:
    std::string recvMsg1;
    std::string recvMsg2;
    CountdownLatch latch(2);

    // when:
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
    PubSub broker;
    broker.Subscribe("/foo", actor1);
    broker.Subscribe("/bar", actor2);
    broker.Broadcast(std::string("bazz"));
    latch.Await();

    // then:
    EXPECT_EQ("bazz", recvMsg1);
    EXPECT_EQ("bazz", recvMsg2);
}

TEST(ActorSystem, Multicast_ForwardMatch) {
    // setup:
    std::string recvMsg1;
    std::string recvMsg2;
    bool ok = true;
    CountdownLatch latch(2);

    // when:
    auto actor_a1 = std::make_shared<Actor>([&](any&& msg) {
        if (msg.type() == typeid(std::string)) {
            recvMsg1 += any_cast<std::string>(msg);
        }
        latch.CountDown();
        return 0;
    });
    auto actor_a2 = std::make_shared<Actor>([&](any&& msg) {
        if (msg.type() == typeid(std::string)) {
            recvMsg2 += any_cast<std::string>(msg);
        }
        latch.CountDown();
        return 0;
    });
    auto actor_b2 = std::make_shared<Actor>([&](any&& msg) {
        ok = false;
        return 0;
    });
    PubSub broker;
    broker.Subscribe("/a/actor/1", actor_a1);
    broker.Subscribe("/a/actor/2", actor_a2);
    broker.Subscribe("/b/actor/2", actor_b2);
    broker.Multicast("/a/actor/#", std::string("foo"));
    latch.Await();

    // then:
    EXPECT_EQ("foo", recvMsg1);
    EXPECT_EQ("foo", recvMsg2);
    EXPECT_TRUE(ok);
}

TEST(ActorSystem, Multicast_PartialMatch) {
    // setup:
    std::string recvMsg1;
    std::string recvMsg2;
    bool ok = true;
    CountdownLatch latch(2);

    // when:
    auto actor_a1 = std::make_shared<Actor>([&](any&& msg) {
        ok = false;
        return 0;
    });
    auto actor_a2 = std::make_shared<Actor>([&](any&& msg) {
        if (msg.type() == typeid(std::string)) {
            recvMsg1 += any_cast<std::string>(msg);
        }
        latch.CountDown();
        return 0;
    });
    auto actor_b2 = std::make_shared<Actor>([&](any&& msg) {
        if (msg.type() == typeid(std::string)) {
            recvMsg2 += any_cast<std::string>(msg);
        }
        latch.CountDown();
        return 0;
    });
    PubSub broker;
    broker.Subscribe("/a/actor/1", actor_a1);
    broker.Subscribe("/a/actor/2", actor_a2);
    broker.Subscribe("/b/actor/2", actor_b2);
    broker.Multicast("/+/actor/2", std::string("foo"));
    latch.Await();

    // then:
    EXPECT_EQ("foo", recvMsg1);
    EXPECT_EQ("foo", recvMsg2);
    EXPECT_TRUE(ok);
}
