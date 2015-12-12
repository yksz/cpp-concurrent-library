#include <future>
#include <iostream>
#include <typeinfo>
#include "ccl/actor.h"
#include "ccl/any.h"

namespace {

struct Point {
    int x;
    int y;
};

} // unnamed namespace

int main(void) {
    auto actor = std::make_shared<ccl::Actor>([](ccl::any& message, std::promise<ccl::any>* promise) {
        const std::type_info& type = message.type();
        if (type == typeid(int)) {
            std::cout << "int: " << ccl::any_cast<int>(message) << std::endl;
        } else if (type == typeid(double)) {
            std::cout << "double: " << ccl::any_cast<double>(message) << std::endl;
        } else if (type == typeid(char)) {
            std::cout << "char: " << ccl::any_cast<char>(message) << std::endl;
        } else if (type == typeid(char*)) {
            std::cout << "char*: " << ccl::any_cast<char*>(message) << std::endl;
            promise->set_value((char*) "char*: ok");
        } else if (type == typeid(std::string)) {
            std::cout << "string: " << ccl::any_cast<std::string>(message) << std::endl;
        } else if (type == typeid(Point)) {
            Point p = ccl::any_cast<Point>(message);
            std::cout << "point: x=" << p.x << ", y=" << p.y << std::endl;
        } else {
            std::cout << "others" << std::endl;
        }
    });
    actor->Send(1);
    actor->Send(2.0);
    actor->Send('3');

    std::future<ccl::any> future1;
    std::future<ccl::any> future2;
    {
        ccl::ActorSystem& system = ccl::ActorSystem::GetInstance();
        system.Register("/path/actor1", actor);
        future1 = system.Send("/path/actor1", (char*) "4");
        future2 = system.Send("/path/actor2", (char*) "5");
        system.Broadcast(std::string("6"));
        system.Broadcast((Point) {7, 8});
        system.Broadcast(9L);
    }
    if (future1.valid()) {
        ccl::any response1 = future1.get();
        if (response1.type() == typeid(char*)) {
            std::cout << "response1: " << ccl::any_cast<char*>(response1) << std::endl;
        }
    }
    if (future2.valid()) {
        std::cout << "response2" << std::endl;
    }
    return 0;
}
