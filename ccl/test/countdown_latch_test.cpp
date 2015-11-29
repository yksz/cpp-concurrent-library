#include "ccl/countdown_latch.h"
#include <chrono>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include <iostream>

using namespace ccl;

TEST(CountdownLatch, AwaitAndCountDown) {
    // setup:
    const int count = 3;

    // when:
    CountdownLatch latch(count);
    std::vector<std::thread> threads;
    for (int i = 0; i < count; i++) {
        threads.emplace_back([&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            latch.CountDown();
        });
    }
    latch.Await();

    // then:
    EXPECT_EQ(0, latch.GetCount());

    // cleanup:
    for (std::thread& th : threads) {
        th.join();
    }
}
