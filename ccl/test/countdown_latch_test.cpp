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

TEST(CountdownLatch, AwaitAndTimeOut) {
    // when:
    CountdownLatch latch;
    std::thread th([&]() {
        latch.Await(std::chrono::milliseconds(100));
        latch.CountDown();
    });
    th.detach();
    latch.Await(std::chrono::milliseconds(10));

    // then:
    EXPECT_EQ(1, latch.GetCount());
}
