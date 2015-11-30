#include "ccl/thread_pool.h"
#include <atomic>
#include <chrono>
#include <thread>
#include <gtest/gtest.h>

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
        ThreadPool pool(10, true);
        for (int i = 0; i < dispatchCount; i++) {
            pool.Dispatch([&]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                count++;
            });
        }
    }

    // then:
    EXPECT_NE(dispatchCount, count);
}
