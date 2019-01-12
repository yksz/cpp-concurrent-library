#include "ccl/continuation.h"
#include <thread>
#include <gtest/gtest.h>
#include "util.h"

using namespace ccl;

TEST(Continuation, Run_Sync) {
    // setup:
    int result = 0;

    // when: create continuations
    auto cont = ccl::Continuation<int>::Make([](int x) {
        return x + 1;
    })
    ->Then([](int x) {
        return x + 1;
    })
    ->Then([&](int x) {
        result = x + 1;
        return result;
    });

    // and: run sync
    cont->Run(1);

    // then:
    EXPECT_EQ(4, result);
}

TEST(Continuation, Run_Async) {
    // setup:
    int result = 0;

    // when: create continuations
    auto cont = ccl::Continuation<int>::Make([](int x) {
        return x + 1;
    })
    ->Then([](int x) {
        return x + 1;
    })
    ->Then([&](int x) {
        result = x + 1;
        return result;
    });

    // and: run async
    std::thread th([=]() {
        cont->Run(1);
    });
    th.join();

    // then:
    EXPECT_EQ(4, result);
}
