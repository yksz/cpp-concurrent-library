#include "ccl/scheduler.h"
#include <ctime>
#include <chrono>
#include <gtest/gtest.h>
#include "ccl/countdown_latch.h"
#include <iostream>

using namespace ccl;
using namespace std::chrono;

TEST(Scheduler, Schedule) {
    // setup:
    auto now = system_clock::from_time_t(time(nullptr));
    auto after1 = now + seconds(1);
    auto after2 = now + seconds(2);
    CountdownLatch latch(2);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(system_clock::to_time_t(after1), [&]() {
        auto newNow = system_clock::from_time_t(time(nullptr));
        auto diff = duration_cast<seconds>(newNow - now);
        // then:
        EXPECT_EQ(1, diff.count());
        latch.CountDown();
    });
    scheduler.Schedule(system_clock::to_time_t(after2), [&]() {
        auto newNow = system_clock::from_time_t(time(nullptr));
        auto diff = duration_cast<seconds>(newNow - now);
        // then:
        EXPECT_EQ(2, diff.count());
        latch.CountDown();
    });
    latch.Await();
}

TEST(ThreadPool, Schedule_loop) {
    // setup:
    auto now = system_clock::from_time_t(time(nullptr));
    auto after = now + seconds(1);
    int count = 1;
    CountdownLatch latch(2);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(system_clock::to_time_t(after), 1000, [&]() {
        auto newNow = system_clock::from_time_t(time(nullptr));
        auto diff = duration_cast<seconds>(newNow - now);
        // then:
        EXPECT_EQ(count++, diff.count());
        latch.CountDown();
    });
    latch.Await();
}
