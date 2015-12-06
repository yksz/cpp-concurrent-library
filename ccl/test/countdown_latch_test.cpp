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
    std::vector<std::thread> threads;
    for (int i = 0; i < count; i++) {
        std::thread th([&]() {
            util::doHeavyTask();
            latch.CountDown();
        });
        th.detach();
        threads.emplace_back(std::move(th));
    }
    latch.Await();

    // then:
    EXPECT_EQ(0, latch.GetCount());
}
