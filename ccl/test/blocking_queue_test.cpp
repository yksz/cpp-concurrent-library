#include "ccl/blocking_queue.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <gtest/gtest.h>
#include "util.h"
#include "ccl/countdown_latch.h"

namespace {

class Object {
public:
    static int s_copyConstructorCount;
    static int s_copyAssignmentCount;

    Object() {
        std::cout << "Object: constructor\n";
    }

    ~Object() {
        std::cout << "Object: destructor\n";
    }

    Object(const Object&) {
        std::cout << "Object: copy constructor\n";
        Object::s_copyConstructorCount++;
    }

    Object& operator=(const Object&) {
        std::cout << "Object: copy assignment\n";
        Object::s_copyAssignmentCount++;
        return *this;
    }

    Object(Object&& other) {
        std::cout << "Object: move constructor\n";
    }

    Object& operator=(Object&&) {
        std::cout << "Object: move assignment\n";
        return *this;
    }
};

int Object::s_copyConstructorCount = 0;
int Object::s_copyAssignmentCount = 0;

} // unnamed namespace

using namespace ccl;

TEST(BlockingQueue, MoveSemantics) {
    // when:
    BlockingQueue<Object> queue;
    std::cout << "Push begin\n";
    queue.Emplace(Object());
    std::cout << "Push end\n";
    Object obj;
    std::cout << "Pop begin\n";
    queue.Pop(obj);
    std::cout << "Pop end\n";

    // then:
    EXPECT_EQ(0, Object::s_copyConstructorCount);
    EXPECT_EQ(0, Object::s_copyAssignmentCount);

    // cleanup:
    Object::s_copyConstructorCount = 0;
    Object::s_copyAssignmentCount = 0;
}

TEST(BlockingQueue, Pop_Blocking) {
    // setup:
    std::string pushedElement = "element";

    // when:
    BlockingQueue<std::string> queue;
    std::thread th([](BlockingQueue<std::string>& queue, const std::string& element) {
        util::doHeavyTask();
        queue.Push(element);
    }, std::ref(queue), std::ref(pushedElement));
    std::string poppedElement = queue.Pop();

    // then:
    EXPECT_EQ(pushedElement, poppedElement);

    // clenup:
    th.join();
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
    util::await();
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

TEST(BlockingQueue, Push_FunctionObject) {
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

TEST(BlockingQueue, Emplace_FunctionObject) {
    // setup:
    int count = 0;

    // when:
    BlockingQueue<std::function<void()>> queue;
    queue.Emplace([&]() {
        count++;
    });

    // and:
    std::function<void()> task;
    queue.Pop(task);
    task();

    // then:
    EXPECT_EQ(1, count);
}

TEST(BlockingQueue, Clear) {
    const int count = 100;

    // when:
    BlockingQueue<int> queue;
    for (int i = 0; i < count; i++) {
        queue.Push(i);
    }

    // and:
    queue.Clear();

    // then:
    EXPECT_TRUE(queue.Empty());
}
