#include "ccl/channel.h"
#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include "util.h"

using namespace ccl;

TEST(Channel, SendAndReceive_MoveSemantics) {
    // when:
    Channel<util::CopyCounter> chan;
    std::thread th([&]() {
        chan.Send(util::CopyCounter{});
    });
    auto counter = chan.Receive();

    // then:
    EXPECT_EQ(0, counter.CopyConstructorCount());
    EXPECT_EQ(1, counter.CopyAssignmentCount());
    EXPECT_EQ(1, counter.MoveConstructorCount());
    EXPECT_EQ(0, counter.MoveAssignmentCount());

    // cleanup:
    th.join();
}

TEST(Channel, SendAndReceiveOperator_MoveSemantics) {
    // when:
    Channel<util::CopyCounter> chan;
    std::thread th([&]() {
        chan << util::CopyCounter{};
    });
    util::CopyCounter counter;
    chan >> counter;

    // then:
    EXPECT_EQ(0, counter.CopyConstructorCount());
    EXPECT_EQ(1, counter.CopyAssignmentCount());
    EXPECT_EQ(1, counter.MoveConstructorCount());
    EXPECT_EQ(1, counter.MoveAssignmentCount());

    // cleanup:
    th.join();
}

TEST(Channel, SendAndReceive) {
    // setup:
    const int sendCount = 10;

    // when:
    Channel<int> chan;
    std::thread th([&]() {
        for (int i = 0; i < sendCount; i++) {
            chan.Send(i);
        }
    });

    // then:
    int received;
    for (int i = 0; i < sendCount; i++) {
        received = chan.Receive();
        EXPECT_EQ(i, received);
    }

    // cleanup:
    th.join();
}
