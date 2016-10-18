#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include "ccl/blocking_queue.h"

static const std::string element = "element";

int main(void) {
    ccl::BlockingQueue<std::string> queue;
    std::thread th([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "push" << std::endl;
        queue.Push(element);
    });
    std::cout << "pop" << std::endl;
    std::cout << queue.Pop() << std::endl;
    th.join();

    // Output:
    // pop
    // push
    // element
    return 0;
}
