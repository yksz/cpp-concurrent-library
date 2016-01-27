#include "ccl/scheduler.h"
#include <ctime>
#include <chrono>
#include <gtest/gtest.h>
#include "ccl/countdown_latch.h"

using namespace ccl;
using namespace std::chrono;

TEST(Scheduler, Schedule) {
    // setup:
    auto now = system_clock::now();
    auto after1 = now + milliseconds(100);
    auto after2 = now + milliseconds(200);
    CountdownLatch latch(2);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(Scheduler::ToUnixTime(after1), [&]() {
        auto newNow = system_clock::now();
        auto diff = duration_cast<milliseconds>(newNow - now);
        // then:
        EXPECT_NEAR(100, diff.count(), 10);
        latch.CountDown();
    });
    scheduler.Schedule(Scheduler::ToUnixTime(after2), [&]() {
        auto newNow = system_clock::now();
        auto diff = duration_cast<milliseconds>(newNow - now);
        // then:
        EXPECT_NEAR(200, diff.count(), 10);
        latch.CountDown();
    });
    latch.Await();
}

TEST(ThreadPool, Schedule_period) {
    // setup:
    auto now = system_clock::now();
    auto after = now + milliseconds(100);
    int count = 1;
    CountdownLatch latch(2);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(Scheduler::ToUnixTime(after), 100, [&]() {
        auto newNow = system_clock::now();
        auto diff = duration_cast<milliseconds>(newNow - now);
        // then:
        EXPECT_NEAR((count++) * 100, diff.count(), 10);
        latch.CountDown();
    });
    latch.Await();
}
