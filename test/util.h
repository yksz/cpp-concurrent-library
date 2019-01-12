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

class CopyCounter {
private:
    int m_copyConstructorCount;
    int m_copyAssignmentCount;
    int m_moveConstructorCount;
    int m_moveAssignmentCount;

public:
    CopyCounter()
            : m_copyConstructorCount(0)
            , m_copyAssignmentCount(0)
            , m_moveConstructorCount(0)
            , m_moveAssignmentCount(0) {}

    ~CopyCounter() = default;

    CopyCounter(const CopyCounter& other) {
        m_copyConstructorCount = other.m_copyConstructorCount + 1;
        m_copyAssignmentCount = other.m_copyAssignmentCount;
        m_moveConstructorCount = other.m_moveConstructorCount;
        m_moveAssignmentCount = other.m_moveAssignmentCount;
    }

    CopyCounter& operator=(const CopyCounter& other) {
        m_copyConstructorCount = other.m_copyConstructorCount;
        m_copyAssignmentCount = other.m_copyAssignmentCount + 1;
        m_moveConstructorCount = other.m_moveConstructorCount;
        m_moveAssignmentCount = other.m_moveAssignmentCount;
        return *this;
    }

    CopyCounter(CopyCounter&& other) {
        m_copyConstructorCount = other.m_copyConstructorCount;
        m_copyAssignmentCount = other.m_copyAssignmentCount;
        m_moveConstructorCount = other.m_moveConstructorCount + 1;
        m_moveAssignmentCount = other.m_moveAssignmentCount;
    }

    CopyCounter& operator=(CopyCounter&& other) {
        m_copyConstructorCount = other.m_copyConstructorCount;
        m_copyAssignmentCount = other.m_copyAssignmentCount;
        m_moveConstructorCount = other.m_moveConstructorCount;
        m_moveAssignmentCount = other.m_moveAssignmentCount + 1;
        return *this;
    }

    int CopyConstructorCount() { return m_copyConstructorCount; }
    int CopyAssignmentCount() { return m_copyAssignmentCount; }
    int MoveConstructorCount() { return m_moveConstructorCount; }
    int MoveAssignmentCount() { return m_moveAssignmentCount; }
};

} // namespace util
