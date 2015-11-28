#include <iostream>
#include <boost/any.hpp>
#include "ccl/actor.h"

int main(void) {
    auto actor = std::make_shared<ccl::Actor>([](const boost::any& message) {
        if (message.type() == typeid(int)) {
            std::cout << "int: " << boost::any_cast<int>(message) << std::endl;
        } else if (message.type() == typeid(double)) {
            std::cout << "double: " << boost::any_cast<double>(message) << std::endl;
        } else if (message.type() == typeid(char)) {
            std::cout << "char: " << boost::any_cast<char>(message) << std::endl;
        } else if (message.type() == typeid(char*)) {
            std::cout << "char*: " << boost::any_cast<char*>(message) << std::endl;
        } else if (message.type() == typeid(std::string)) {
            std::cout << "string: " << boost::any_cast<std::string>(message) << std::endl;
        } else {
            std::cout << "others" << std::endl;
        }
    });
    actor->Tell(1);
    actor->Tell(2.0);
    actor->Tell('3');

    ccl::ActorSystem& system = ccl::ActorSystem::GetInstance();
    system.Register("/path/actor1", actor);
    system.Send("/path/actor1", (char*) "4");
    system.Send("/path/actor2", (char*) "5");
    system.Broadcast(std::string("6"));
    return 0;
}
