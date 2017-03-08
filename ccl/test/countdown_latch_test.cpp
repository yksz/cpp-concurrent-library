#include "ccl/countdown_latch.h"
#include <chrono>
#include <thread>
#include <vector>
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
            util::doHeavyTask();
            latch.CountDown();
        });
        th.detach();
    }
    latch.Await();

    // then:
    EXPECT_EQ(0, latch.GetCount());
}

TEST(CountdownLatch, Await_Timeout) {
    // when:
    CountdownLatch latch;
    std::thread th([&]() {
        util::doHeavyTask();
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
