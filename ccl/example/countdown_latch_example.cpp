#include <iostream>
#include <mutex>
#include <thread>
#include "ccl/countdown_latch.h"

static const int kNthreads = 5;

int main(void) {
    ccl::CountdownLatch latch(kNthreads);
    std::mutex mutex;
    for (int i = 0; i < kNthreads; i++) {
        std::thread th([&]() {
            {
                std::lock_guard<std::mutex> lock(mutex);
                std::cout << latch.GetCount() << std::endl;
            }
            latch.CountDown();
        });
        th.detach();
    }
    latch.Await();
    std::cout << latch.GetCount() << std::endl;
    std::cout << "finish" << std::endl;

    // Output:
    // 5
    // 4
    // 3
    // 2
    // 1
    // 0
    // finish
    return 0;
}
