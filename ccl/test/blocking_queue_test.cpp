#include "ccl/blocking_queue.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <gtest/gtest.h>

namespace {

class Object {
public:
    static int m_copyConstructorCount;
    static int m_copyAssignmentCount;

    Object() {
        std::cout << "Object: constructor\n";
    }

    ~Object() {
        std::cout << "Object: destructor\n";
    }

    Object(const Object&) {
        std::cout << "Object: copy constructor\n";
        Object::m_copyConstructorCount++;
    }

    Object& operator=(const Object&) {
        std::cout << "Object: copy assignment\n";
        Object::m_copyAssignmentCount++;
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

int Object::m_copyConstructorCount = 0;
int Object::m_copyAssignmentCount = 0;

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
    EXPECT_EQ(0, Object::m_copyConstructorCount);
    EXPECT_EQ(0, Object::m_copyAssignmentCount);

    // cleanup:
    Object::m_copyConstructorCount = 0;
    Object::m_copyAssignmentCount = 0;
}

TEST(BlockingQueue, Pop_Blocking) {
    // setup:
    std::string pushedElement = "element";

    // when:
    BlockingQueue<std::string> queue;
    std::thread th([](BlockingQueue<std::string>& queue, const std::string& element) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        queue.Push(element);
    }, std::ref(queue), std::ref(pushedElement));
    std::string poppedElement = queue.Pop();

    // then:
    EXPECT_EQ(pushedElement, poppedElement);

    // clenup:
    th.join();
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
