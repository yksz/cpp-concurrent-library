#include <cstdio>
#include <ctime>
#include <chrono>
#include <iostream>
#include "ccl/scheduler.h"

static const int kPeriod = 1000; // [ms]
static const int kRepeatCount = 5;

int main(void) {
    using namespace std::chrono;

    int sec = time(nullptr) % 60;
    int waitTime = (10 - sec % 10) * 1000; // [ms]
    auto firstTime = system_clock::now() + milliseconds(waitTime); // at hh:mm:s0
    auto period = milliseconds(kPeriod); // [ms]

    ccl::Scheduler scheduler;
    scheduler.Schedule(firstTime, period, kRepeatCount, []() {
        time_t now = time(nullptr);
        char s[16];
        strftime(s, sizeof(s), "%H:%M:%S", localtime(&now));
        printf("%s\n", s);
    });

    long sleepTime = kPeriod * (kRepeatCount + 1) + waitTime; // [ms]
    std::this_thread::sleep_for(milliseconds(sleepTime));

    // Output:
    // hh:mm:s0
    // hh:mm:s1
    // hh:mm:s2
    // hh:mm:s3
    // hh:mm:s4
    // hh:mm:s5
    return 0;
}
