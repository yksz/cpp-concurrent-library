#include <iostream>
#include <mutex>
#include "ccl/thread_pool.h"

static const int kNthreads = 2;
static const int kDispatchCount = 5;

int main(void) {
    std::mutex mutex;
    int count = 0;

    {
        ccl::ThreadPool pool(kNthreads);
        for (int i = 0; i < kDispatchCount; i++) {
            pool.Dispatch([&]() {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    std::cout << "Thread_" << std::this_thread::get_id()
                              << ": count=" << count << std::endl;
                    count++;
                }
            });
        }
    }
    // wait for all tasks to complete

    // Output:
    // Thread_<ID>: count=0
    // Thread_<ID>: count=1
    // Thread_<ID>: count=2
    // Thread_<ID>: count=3
    // Thread_<ID>: count=4
    return 0;
}
