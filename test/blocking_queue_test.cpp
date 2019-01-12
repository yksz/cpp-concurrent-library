#include "ccl/blocking_queue.h"
#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include "ccl/countdown_latch.h"
#include "util.h"

using namespace ccl;

TEST(BlockingQueue, PushLvalueAndPop_MoveSemantics) {
    // when:
    BlockingQueue<util::CopyCounter> queue;
    util::CopyCounter pushed;
    queue.Push(pushed);
    auto counter = queue.Pop();

    // then:
    EXPECT_EQ(1, counter.CopyConstructorCount());
    EXPECT_EQ(0, counter.CopyAssignmentCount());
    EXPECT_EQ(0, counter.MoveConstructorCount());
    EXPECT_EQ(1, counter.MoveAssignmentCount());
}

TEST(BlockingQueue, PushRvalueAndPop_MoveSemantics) {
    // when:
    BlockingQueue<util::CopyCounter> queue;
    queue.Push(util::CopyCounter{});
    auto counter = queue.Pop();

    // then:
    EXPECT_EQ(0, counter.CopyConstructorCount());
    EXPECT_EQ(0, counter.CopyAssignmentCount());
    EXPECT_EQ(1, counter.MoveConstructorCount());
    EXPECT_EQ(1, counter.MoveAssignmentCount());
}

TEST(BlockingQueue, Push_Blocking) {
    // setup:
    const int capacity = 5;
    CountdownLatch latch(capacity);

    // when:
    BlockingQueue<int> queue(capacity);
    for (int i = 0; i < capacity + 1; i++) {
        std::thread th([&]() {
            queue.Push(i);
            latch.CountDown();
        });
        th.detach();
    }
    latch.Await();

    // then:
    EXPECT_EQ(capacity, queue.Size());
    queue.Pop();
    queue.Pop();
    util::Delay();
    EXPECT_EQ(capacity - 1, queue.Size());
}

TEST(BlockingQueue, Push_NonBlocking) {
    // setup:
    const int count = 100;
    CountdownLatch latch(count);

    // when:
    BlockingQueue<int> queue;
    for (int i = 0; i < count; i++) {
        std::thread th([&]() {
            queue.Push(i);
            latch.CountDown();
        });
        th.detach();
    }
    latch.Await();

    // then:
    EXPECT_EQ(count, queue.Size());
}

TEST(BlockingQueue, Pop_Blocking) {
    // setup:
    const std::string pushed = "element";

    // when:
    BlockingQueue<std::string> queue;
    std::thread th([](BlockingQueue<std::string>& queue, const std::string& element) {
        util::DoHeavyTask();
        queue.Push(element);
    }, std::ref(queue), std::ref(pushed));
    std::string popped = queue.Pop();

    // then:
    EXPECT_EQ(pushed, popped);

    // clenup:
    th.join();
}

TEST(BlockingQueue, Push_Timeout) {
    // when:
    BlockingQueue<int> queue(1);
    queue.Push(1);
    std::cv_status status = queue.Push(2, std::chrono::milliseconds(10));

    // then:
    EXPECT_EQ(std::cv_status::timeout, status);
}

TEST(BlockingQueue, Push_NoTimeout) {
    // when:
    BlockingQueue<int> queue;
    std::cv_status status = queue.Push(1, std::chrono::seconds(10));

    // then:
    EXPECT_EQ(std::cv_status::no_timeout, status);
}

TEST(BlockingQueue, Pop_Timeout) {
    // when:
    BlockingQueue<int> queue;
    int unused;
    std::cv_status status = queue.Pop(std::chrono::milliseconds(10), &unused);

    // then:
    EXPECT_EQ(std::cv_status::timeout, status);
}

TEST(BlockingQueue, Pop_NoTimeout) {
    // setup:
    int pushed = 1;

    // when:
    BlockingQueue<int> queue;
    queue.Push(pushed);

    // and:
    int popped;
    std::cv_status status = queue.Pop(std::chrono::seconds(10), &popped);

    // then:
    EXPECT_EQ(std::cv_status::no_timeout, status);
    EXPECT_EQ(pushed, popped);
}

TEST(BlockingQueue, PushLvalue_FunctionObject) {
    // setup:
    int count = 0;

    // when:
    BlockingQueue<std::function<void()>> queue;
    {
        std::function<void()> task = [&]() { count++; };
        queue.Push(task);
    }

    // and:
    std::function<void()> task = queue.Pop();
    task();

    // then:
    EXPECT_EQ(1, count);
}

TEST(BlockingQueue, PushRvalue_FunctionObject) {
    // setup:
    int count = 0;

    // when:
    BlockingQueue<std::function<void()>> queue;
    queue.Push([&]() {
        count++;
    });

    // and:
    std::function<void()> task = queue.Pop();
    task();

    // then:
    EXPECT_EQ(1, count);
}

TEST(BlockingQueue, Clear) {
    // setup:
    const int count = 100;
    const int last = -1;

    // when: push until capcity is full
    BlockingQueue<int> queue(count);
    for (int i = 0; i < count; i++) {
        queue.Push(i);
    }

    // and: wait until popped
    std::thread th([&]() {
        queue.Push(last);
    });

    // and: wait until pushed the last element and clear
    util::Delay();
    queue.Clear();
    th.join();

    // then:
    EXPECT_EQ(last, queue.Pop());
    EXPECT_TRUE(queue.Empty());
}
