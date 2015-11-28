#include "ccl/thread_pool.h"
#include <atomic>
#include <thread>
#include <gtest/gtest.h>

using namespace ccl;

TEST(ThreadPool, Dispatch) {
    // setup:
    const int dispatchCount = 100;
    std::atomic<int> count(0);

    // when:
    {
        ThreadPool pool(10);
        for (int i = 0; i < dispatchCount; i++) {
            pool.Dispatch([&]() { count++; });
        }
    }

    // then:
    EXPECT_EQ(dispatchCount, count);

    // clenup:
    count = 0;
}
