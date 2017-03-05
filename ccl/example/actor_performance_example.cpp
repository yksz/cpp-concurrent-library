#include <future>
#include <iostream>
#include <typeinfo>
#include <vector>
#include "ccl/actor.h"

namespace {

class Object {
private:
    std::string m_name;

public:
    Object(std::string name) : m_name(name) {
        std::cout << "Object: constructor" << std::endl;
    }

    ~Object() {
        std::cout << "Object: destructor" << std::endl;
    }

    Object(const Object& o) {
        std::cout << "Object: copy constructor" << std::endl;
        m_name = o.m_name;
    }

    Object& operator=(const Object& o) {
        std::cout << "Object: copy assignment" << std::endl;
        m_name = o.m_name;
        return *this;
    }

    Object(Object&& o) {
        std::cout << "Object: move constructor" << std::endl;
        m_name = std::move(o.m_name);
    }

    Object& operator=(Object&& o) {
        std::cout << "Object: move assignment" << std::endl;
        m_name = std::move(o.m_name);
        return *this;
    }

    std::string& GetName() {
        return m_name;
    }
};

} // namespace

int main(void) {
    auto actor = std::make_shared<ccl::Actor>([](ccl::any&& msg) -> ccl::any {
        const std::type_info& type = msg.type();
        if (type == typeid(std::shared_ptr<Object>)) {
            auto obj = ccl::any_cast<std::shared_ptr<Object>>(msg);
            std::cout << "Object: name=" << obj->GetName() << std::endl;
        } else if (type == typeid(Object)) {
            Object& obj = ccl::any_cast<Object&>(msg);
            std::cout << "Object: name=" << obj.GetName() << std::endl;
        } else {
            std::cout << "others" << std::endl;
        }
        return 0;
    });

    {
        std::cout << "### Better performance" << std::endl;
        auto obj = std::make_shared<Object>("shared_ptr");
        auto future = actor->Send(obj);
        future.get();
    }
    std::cout << std::endl;
    {
        std::cout << "### Worse performance" << std::endl;
        ccl::any msg(Object("not shared_ptr"));
        auto future = actor->Send(msg);
        future.get();
    }
    return 0;
}
