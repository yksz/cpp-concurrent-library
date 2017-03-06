#include "ccl/blocking_queue.h"
#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include "ccl/countdown_latch.h"
#include "util.h"

namespace {

class Object {
public:
    static int copyConstructorCount;
    static int copyAssignmentCount;
    static int moveConstructorCount;
    static int moveAssignmentCount;

    static void ClearCounts() {
        Object::copyConstructorCount = 0;
        Object::copyAssignmentCount = 0;
        Object::moveConstructorCount = 0;
        Object::moveAssignmentCount = 0;
    }

    Object() = default;
    ~Object() = default;

    Object(const Object&) {
        Object::copyConstructorCount++;
    }

    Object& operator=(const Object&) {
        Object::copyAssignmentCount++;
        return *this;
    }

    Object(Object&&) {
        Object::moveConstructorCount++;
    }

    Object& operator=(Object&&) {
        Object::moveAssignmentCount++;
        return *this;
    }

    void DoNothing() {}
};

int Object::copyConstructorCount = 0;
int Object::copyAssignmentCount = 0;
int Object::moveConstructorCount = 0;
int Object::moveAssignmentCount = 0;

} // namespace

using namespace ccl;

TEST(BlockingQueue, PushLvalue_MoveSemantics) {
    // setup:
    BlockingQueue<Object> queue;
    Object::ClearCounts();

    // when:
    Object obj;
    queue.Push(obj);

    // then:
    EXPECT_EQ(1, Object::copyConstructorCount);
    EXPECT_EQ(0, Object::moveConstructorCount);
}

TEST(BlockingQueue, PushRvalue_MoveSemantics) {
    // setup:
    BlockingQueue<Object> queue;
    Object::ClearCounts();

    // when:
    queue.Push(Object{});

    // then:
    EXPECT_EQ(0, Object::copyConstructorCount);
    EXPECT_EQ(1, Object::moveConstructorCount);
}

TEST(BlockingQueue, Pop_MoveSemantics) {
    // setup:
    BlockingQueue<Object> queue;
    queue.Push(Object{});
    Object::ClearCounts();

    // when:
    Object obj = queue.Pop();
    obj.DoNothing();

    // then:
    EXPECT_EQ(0, Object::copyAssignmentCount);
    EXPECT_EQ(1, Object::moveAssignmentCount);
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

TEST(BlockingQueue, Pop_Blocking) {
    // setup:
    const std::string pushed = "element";

    // when:
    BlockingQueue<std::string> queue;
    std::thread th([](BlockingQueue<std::string>& queue, const std::string& element) {
        util::doHeavyTask();
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
