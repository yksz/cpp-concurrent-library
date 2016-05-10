#include <cstdio>
#include <ctime>
#include <chrono>
#include <iostream>
#include "ccl/scheduler.h"

int main(void) {
    using namespace std::chrono;

    ccl::Scheduler scheduler;
    auto after1s = system_clock::now() + seconds(1);
    scheduler.SchedulePeriodically(ccl::Scheduler::ToUnixTime(after1s), 1000, [=]() {
        char s[16];
        time_t now = time(nullptr);
        strftime(s, sizeof(s), "%H:%M:%S", localtime(&now));
        printf("%s\n", s);
    });
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
