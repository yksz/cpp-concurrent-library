#pragma once

#include <functional>
#include <memory>

namespace ccl {

template<typename T>
class Continuation final {
private:
    const std::function<T(T)> m_func;

public:
    static std::shared_ptr<Continuation<T>> Make(std::function<T(T)> k) {
        return std::make_shared<Continuation<T>>(k);
    }

    explicit Continuation(std::function<T(T)> func) : m_func(func) {}

    ~Continuation() = default;
    Continuation(const Continuation&) = delete;
    Continuation& operator=(const Continuation&) = delete;

    T Run(T a) {
        return m_func(a);
    }

    std::shared_ptr<Continuation<T>> Then(std::function<T(T)> k) {
        auto& func = m_func;
        return Continuation<T>::Make([func, k](T a) { return k(func(a)); });
    }
};

} // namespace ccl
