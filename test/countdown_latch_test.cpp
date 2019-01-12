#include "ccl/countdown_latch.h"
#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include "util.h"

using namespace ccl;

TEST(CountdownLatch, AwaitAndCountDown) {
    // setup:
    const int count = 3;

    // when:
    CountdownLatch latch(count);
    for (int i = 0; i < count; i++) {
        std::thread th([&]() {
            util::DoHeavyTask();
            latch.CountDown();
        });
        th.detach();
    }
    latch.Await();

    // then:
    EXPECT_EQ(0, latch.GetCount());
}

TEST(CountdownLatch, AwaitAndCountDown_Gate) {
    // setup:
    const int nthreads = 10;

    // when:
    CountdownLatch startGate(1);
    CountdownLatch endGate(nthreads);
    for (int i = 0; i < nthreads; i++) {
        std::thread th([&]() {
            startGate.Await();
            endGate.CountDown();
        });
        th.detach();
    }

    // and: start and await the threads
    startGate.CountDown();
    endGate.Await();

    // then:
    EXPECT_EQ(0, startGate.GetCount());
    EXPECT_EQ(0, endGate.GetCount());
}

TEST(CountdownLatch, Await_Timeout) {
    // when:
    CountdownLatch latch;
    std::thread th([&]() {
        util::DoHeavyTask();
        latch.CountDown();
    });
    th.detach();
    std::cv_status status = latch.Await(std::chrono::milliseconds(10));

    // then:
    EXPECT_EQ(1, latch.GetCount());
    EXPECT_EQ(std::cv_status::timeout, status);
}

TEST(CountdownLatch, Await_NoTimeout) {
    // when:
    CountdownLatch latch;
    std::thread th([&]() {
        latch.CountDown();
    });
    th.detach();
    std::cv_status status = latch.Await(std::chrono::seconds(10));

    // then:
    EXPECT_EQ(0, latch.GetCount());
    EXPECT_EQ(std::cv_status::no_timeout, status);
}
