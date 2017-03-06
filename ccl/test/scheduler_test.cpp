#include "ccl/scheduler.h"
#include <ctime>
#include <chrono>
#include <gtest/gtest.h>
#include "ccl/countdown_latch.h"

using namespace ccl;
using namespace std::chrono;

TEST(Scheduler, Schedule) {
    // setup:
    const int after1 =  50;
    const int after2 = 100;
    auto now = system_clock::now();
    auto startTime1 = now + milliseconds(after1);
    auto startTime2 = now + milliseconds(after2);
    CountdownLatch latch(2);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(Scheduler::ToUnixTime(startTime1), [&]() {
        auto newNow = system_clock::now();
        auto diff = duration_cast<milliseconds>(newNow - now);
        // then:
        EXPECT_NEAR(after1, diff.count(), after1 * 0.1);
        latch.CountDown();
    });
    scheduler.Schedule(Scheduler::ToUnixTime(startTime2), [&]() {
        auto newNow = system_clock::now();
        auto diff = duration_cast<milliseconds>(newNow - now);
        // then:
        EXPECT_NEAR(after2, diff.count(), after2 * 0.1);
        latch.CountDown();
    });
    latch.Await();
}

TEST(Scheduler, SchedulePeriodically) {
    // setup:
    const int period = 50;
    const int repeatCount = 2;
    auto now = system_clock::now();
    int count = 0;
    CountdownLatch latch(repeatCount + 1);

    // when:
    Scheduler scheduler;
    scheduler.SchedulePeriodically(Scheduler::ToUnixTime(now), period, repeatCount, [&]() {
        auto newNow = system_clock::now();
        auto diff = duration_cast<milliseconds>(newNow - now);
        // then:
        EXPECT_NEAR(count * period, diff.count(), period * 0.1);
        count++;
        latch.CountDown();
    });
    latch.Await();
}
