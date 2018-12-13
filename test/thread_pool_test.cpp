#include "ccl/thread_pool.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <set>
#include <thread>
#include <gtest/gtest.h>
#include "util.h"

using namespace ccl;

TEST(ThreadPool, Dispatch) {
    // setup:
    const int nthreads = 10;
    const int dispatchCount = 1000;
    std::mutex mutex;
    std::set<std::thread::id> threadIDs;
    int count = 0;

    // when:
    {
        ThreadPool pool(nthreads);
        for (int i = 0; i < dispatchCount; i++) {
            pool.Dispatch([&]() {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    threadIDs.insert(std::this_thread::get_id());
                    count++;
                }
            });
        }
    }

    // then:
    EXPECT_GE(nthreads, threadIDs.size());
    EXPECT_EQ(dispatchCount, count);
}

TEST(ThreadPool, ShutdownNow) {
    // setup:
    const int nthreads = 10;
    const int dispatchCount = 1000;
    std::atomic<int> count(0);

    // when:
    {
        ThreadPool pool(nthreads);
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
