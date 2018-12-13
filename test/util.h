#pragma once

#include <chrono>
#include <thread>

namespace util {

inline void await() {
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

inline void doHeavyTask() {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

} // namespace util
