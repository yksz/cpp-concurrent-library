#include <iostream>
#include <thread>
#include "ccl/continuation.h"

int main(void) {
    auto cont = ccl::Continuation<int>::Make([](int x) {
        int result = x + 1;
        std::cout << "make: " << x << " -> " << result << std::endl;
        return result;
    })
    ->Then([](int x) {
        int result = x + 1;
        std::cout << "then: " << x << " -> " << result << std::endl;
        return result;
    })
    ->Then([](int x) {
        int result = x + 1;
        std::cout << "then: " << x << " -> " << result << std::endl;
        return result;
    });

    std::thread th([=]() {
        cont->Run(1);
    });
    th.join();

    // Output:
    // make: 1 -> 2
    // then: 2 -> 3
    // then: 3 -> 4
    return 0;
}
