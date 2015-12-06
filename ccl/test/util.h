#pragma once

#include <chrono>
#include <thread>

namespace util {

void await() {
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void doHeavyTask() {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

} // namespace util
