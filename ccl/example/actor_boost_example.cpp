#include <future>
#include <iostream>
#include <typeinfo>
#include "ccl/actor_boost.h"

int main(void) {
    auto actor = std::make_shared<ccl::Actor>([](const boost::any& message) -> boost::any {
        const std::type_info& type = message.type();
        if (type == typeid(int)) {
            std::cout << "int: " << boost::any_cast<int>(message) << std::endl;
        } else if (type == typeid(double)) {
            std::cout << "double: " << boost::any_cast<double>(message) << std::endl;
        } else {
            std::cout << "others" << std::endl;
        }
        return 0;
    });
    actor->Send(1);
    actor->Send(2.0);
    actor->Send('3');
    return 0;
}
