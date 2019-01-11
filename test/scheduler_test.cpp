#include "ccl/scheduler.h"
#include <ctime>
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include "ccl/countdown_latch.h"

using namespace ccl;
using namespace std::chrono;

const double kScheduleErrorRatio = 0.2;

TEST(Scheduler, Schedule) {
    // setup:
    const int after1 = 30;
    const int after2 = 60;
    const auto baseTime = system_clock::now();
    const auto startTime1 = baseTime + milliseconds(after1);
    const auto startTime2 = baseTime + milliseconds(after2);
    CountdownLatch latch(2);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(startTime1, [&]() {
        auto now = system_clock::now();
        auto diff = duration_cast<milliseconds>(now - baseTime);
        // then:
        int expected = after1;
        int error = expected * kScheduleErrorRatio;
        EXPECT_NEAR(expected, diff.count(), error);
        latch.CountDown();
    });
    scheduler.Schedule(startTime2, [&]() {
        auto now = system_clock::now();
        auto diff = duration_cast<milliseconds>(now - baseTime);
        // then:
        int expected = after2;
        int error = expected * kScheduleErrorRatio;
        EXPECT_NEAR(expected, diff.count(), error);
        latch.CountDown();
    });
    latch.Await();
}

TEST(Scheduler, Schedule_Periodically) {
    // setup:
    const int repeatCount = 2;
    const auto firstTime = system_clock::now();
    const auto period = milliseconds(30);
    int count = 0;
    CountdownLatch latch(repeatCount + 1);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(firstTime, period, repeatCount, [&]() {
        auto now = system_clock::now();
        auto diff = duration_cast<milliseconds>(now - firstTime);
        // then:
        int expected = count * period.count();
        int error = expected * kScheduleErrorRatio;
        EXPECT_NEAR(expected, diff.count(), error);
        count++;
        latch.CountDown();
    });
    latch.Await();
}

TEST(Scheduler, SchedulePeriodically_Forever) {
    // setup:
    const int repeatCount = -1; // endless
    const auto firstTime = system_clock::now();
    const auto period = milliseconds(5);
    const int stopCount = 10;
    std::atomic<int> count(0);
    CountdownLatch latch(stopCount);

    // when:
    {
        Scheduler scheduler;
        scheduler.Schedule(firstTime, period, repeatCount, [&]() {
            count++;
            latch.CountDown();
        });
        latch.Await();
    }
    // then:
    EXPECT_EQ(stopCount, count);
}

TEST(Scheduler, Cancel) {
    // setup:
    const int repeatCount = 5;
    const auto firstTime = system_clock::now();
    const auto period = milliseconds(1);
    std::atomic<int> count(0);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(firstTime, period, repeatCount, [&]() {
        count++;
    });
    scheduler.Cancel();
    std::this_thread::sleep_for(milliseconds(repeatCount));

    // then:
    EXPECT_GT(repeatCount, count);
}
