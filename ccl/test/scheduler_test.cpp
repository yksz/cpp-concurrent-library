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
    auto baseTime = system_clock::now();
    auto startTime1 = baseTime + milliseconds(after1);
    auto startTime2 = baseTime + milliseconds(after2);
    CountdownLatch latch(2);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(Scheduler::ToUnixTime(startTime1), [&]() {
        auto now = system_clock::now();
        auto diff = duration_cast<milliseconds>(now - baseTime);
        // then:
        EXPECT_NEAR(after1, diff.count(), after1 * 0.1);
        latch.CountDown();
    });
    scheduler.Schedule(Scheduler::ToUnixTime(startTime2), [&]() {
        auto now = system_clock::now();
        auto diff = duration_cast<milliseconds>(now - baseTime);
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
    auto firstTime = system_clock::now();
    int count = 0;
    CountdownLatch latch(repeatCount + 1);

    // when:
    Scheduler scheduler;
    scheduler.SchedulePeriodically(Scheduler::ToUnixTime(firstTime), period, repeatCount, [&]() {
        auto now = system_clock::now();
        auto diff = duration_cast<milliseconds>(now - firstTime);
        // then:
        EXPECT_NEAR(count * period, diff.count(), period * 0.1);
        count++;
        latch.CountDown();
    });
    latch.Await();
}

TEST(Scheduler, SchedulePeriodically_Forever) {
    // setup:
    const int period = 5;
    const int repeatCount = -1; // endless
    auto firstTime = system_clock::now();
    const int stopCount = 10;
    int count = 0;
    CountdownLatch latch(stopCount);

    // when:
    {
        Scheduler scheduler;
        scheduler.SchedulePeriodically(Scheduler::ToUnixTime(firstTime), period, repeatCount, [&]() {
            count++;
            latch.CountDown();
        });
        latch.Await();
    }
    // then:
    EXPECT_EQ(stopCount, count);
}
