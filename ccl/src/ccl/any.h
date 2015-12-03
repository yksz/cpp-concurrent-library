// See http://www.boost.org/libs/any for Documentation.

# pragma once

// what:  variant type boost::any
// who:   contributed by Kevlin Henney,
//        with features contributed and bugs found by
//        Antony Polukhin, Ed Brey, Mark Rodgers,
//        Peter Dimov, and James Curran
// when:  July 2001, Aplril 2013

#include <algorithm>
#include <typeinfo>
#include <type_traits>

namespace ccl {

class any {
public: // structors
    any() : content(0) {}

    template<typename ValueType>
    any(const ValueType& value) : content(new holder<ValueType>(value)) {}

    any(const any& other) : content(other.content ? other.content->clone() : 0) {}

    // Move constructor
    any(any&& other) : content(other.content) {
        other.content = 0;
    }

    // Perfect forwarding of ValueType
    template<typename ValueType>
    any(ValueType&& value,
            typename std::enable_if<!std::is_same<any&, ValueType>::value>::type* = 0, // disable if value has type `any&`
            typename std::enable_if<!std::is_const<ValueType>::value>::type* = 0) // disable if value has type `const ValueType&&`
            : content(new holder<typename std::decay<ValueType>::type>(static_cast<ValueType&&>(value))) {}

    ~any() {
        delete content;
    }

public: // modifiers
    any& swap(any& rhs) {
        std::swap(content, rhs.content);
        return *this;
    }

    any& operator=(const any& rhs) {
        any(rhs).swap(*this);
        return *this;
    }

    // move assignement
    any& operator=(any&& rhs) {
        rhs.swap(*this);
        any().swap(rhs);
        return *this;
    }

    // Perfect forwarding of ValueType
    template <class ValueType>
    any& operator=(ValueType&& rhs) {
        any(static_cast<ValueType&&>(rhs)).swap(*this);
        return *this;
    }

public: // queries
    bool empty() const {
        return !content;
    }

    void clear() {
        any().swap(*this);
    }

    const std::type_info& type() const {
        return content ? content->type() : typeid(void);
    }

private: // types
    class placeholder {
    public: // structors
        virtual ~placeholder() {}

    public: // queries
        virtual const std::type_info& type() const = 0;
        virtual placeholder* clone() const = 0;
    };

    template<typename ValueType>
    class holder : public placeholder {
    public: // structors
        holder(const ValueType& value) : held(value) {}
        holder(ValueType&& value) : held(static_cast<ValueType&&>(value)) {}

    public: // queries
        virtual const std::type_info& type() const {
            return typeid(ValueType);
        }

        virtual placeholder* clone() const {
            return new holder(held);
        }

    public: // representation
        ValueType held;

    private: // intentionally left unimplemented
        holder& operator=(const holder&);
    };

private: // representation
    template<typename ValueType>
    friend ValueType* any_cast(any*);

    placeholder* content;
};

inline void swap(any& lhs, any& rhs) {
    lhs.swap(rhs);
}

class bad_any_cast : public std::bad_cast {
public:
    virtual const char* what() const throw() {
        return "bad_any_cast: failed conversion using any_cast";
    }
};

template<typename ValueType>
ValueType* any_cast(any* operand) {
    return operand && operand->type() == typeid(ValueType) ? &static_cast<any::holder<ValueType>*>(operand->content)->held : 0;
}

template<typename ValueType>
inline const ValueType* any_cast(const any* operand) {
    return any_cast<ValueType>(const_cast<any*>(operand));
}

template<typename ValueType>
ValueType any_cast(any& operand) {
    typedef typename std::remove_reference<ValueType>::type nonref;
    nonref* result = any_cast<nonref>(&operand);
    if (!result)
        throw bad_any_cast();
    return *result;
}

template<typename ValueType>
inline ValueType any_cast(const any& operand) {
    typedef typename std::remove_reference<ValueType>::type nonref;
    return any_cast<const nonref&>(const_cast<any&>(operand));
}

template<typename ValueType>
inline ValueType any_cast(any&& operand) {
    static_assert(std::is_rvalue_reference<ValueType&&>::value // true if ValueType is rvalue or just a value
            || std::is_const<typename std::remove_reference<ValueType>::type>::value,
            "any_cast shall not be used for getting nonconst references to temporary objects");
    return any_cast<ValueType>(operand);
}

} // namespace ccl

// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
