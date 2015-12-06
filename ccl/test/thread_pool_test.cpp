#include "ccl/thread_pool.h"
#include <atomic>
#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include "util.h"

using namespace ccl;

TEST(ThreadPool, Dispatch) {
    // setup:
    const int dispatchCount = 1000;
    std::atomic<int> count(0);

    // when:
    {
        ThreadPool pool(10);
        for (int i = 0; i < dispatchCount; i++) {
            pool.Dispatch([&]() {
                count++;
            });
        }
    }

    // then:
    EXPECT_EQ(dispatchCount, count);
}

TEST(ThreadPool, ShutdownNow) {
    // setup:
    const int dispatchCount = 1000;
    std::atomic<int> count(0);

    // when:
    {
        ThreadPool pool(10);
        pool.SetShutdownNow(true);
        for (int i = 0; i < dispatchCount; i++) {
            pool.Dispatch([&]() {
                util::doHeavyTask();
                count++;
            });
        }
    }

    // then:
    EXPECT_NE(dispatchCount, count);
}
