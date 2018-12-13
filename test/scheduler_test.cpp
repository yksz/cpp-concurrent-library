#include "ccl/scheduler.h"
#include <ctime>
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include "ccl/countdown_latch.h"

using namespace ccl;
using namespace std::chrono;

TEST(Scheduler, Schedule) {
    // setup:
    const int after1 = 30;
    const int after2 = 60;
    auto baseTime = system_clock::now();
    auto startTime1 = baseTime + milliseconds(after1);
    auto startTime2 = baseTime + milliseconds(after2);
    CountdownLatch latch(2);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(startTime1, [&]() {
        auto now = system_clock::now();
        auto diff = duration_cast<milliseconds>(now - baseTime);
        // then:
        EXPECT_NEAR(after1, diff.count(), after1 * 0.1);
        latch.CountDown();
    });
    scheduler.Schedule(startTime2, [&]() {
        auto now = system_clock::now();
        auto diff = duration_cast<milliseconds>(now - baseTime);
        // then:
        EXPECT_NEAR(after2, diff.count(), after2 * 0.1);
        latch.CountDown();
    });
    latch.Await();
}

TEST(Scheduler, Schedule_Periodically) {
    // setup:
    const int repeatCount = 2;
    auto firstTime = system_clock::now();
    auto period = milliseconds(30);
    int count = 0;
    CountdownLatch latch(repeatCount + 1);

    // when:
    Scheduler scheduler;
    scheduler.Schedule(firstTime, period, repeatCount, [&]() {
        auto now = system_clock::now();
        auto diff = duration_cast<milliseconds>(now - firstTime);
        // then:
        EXPECT_NEAR(count * period.count(), diff.count(), period.count() * 0.1);
        count++;
        latch.CountDown();
    });
    latch.Await();
}

TEST(Scheduler, SchedulePeriodically_Forever) {
    // setup:
    const int repeatCount = -1; // endless
    auto firstTime = system_clock::now();
    auto period = milliseconds(5);
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
    auto firstTime = system_clock::now();
    auto period = milliseconds(1);
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
