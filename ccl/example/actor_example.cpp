#include <future>
#include <iostream>
#include <typeinfo>
#define CCL_ACTOR_DEBUG
#include "ccl/actor.h"

namespace {

struct Point {
    int x;
    int y;
};

} // unnamed namespace

int main(void) {
    auto actor = std::make_shared<ccl::Actor>([](ccl::any&& msg) -> ccl::any {
        const std::type_info& type = msg.type();
        if (type == typeid(int)) {
            std::cout << "int: " << ccl::any_cast<int>(msg) << std::endl;
        } else if (type == typeid(double)) {
            std::cout << "double: " << ccl::any_cast<double>(msg) << std::endl;
        } else if (type == typeid(char)) {
            std::cout << "char: " << ccl::any_cast<char>(msg) << std::endl;
        } else if (type == typeid(char*)) {
            std::cout << "char*: " << ccl::any_cast<char*>(msg) << std::endl;
            return (char*) "char*: ok";
        } else if (type == typeid(std::string)) {
            std::cout << "string: " << ccl::any_cast<std::string>(msg) << std::endl;
        } else if (type == typeid(Point)) {
            Point& p = ccl::any_cast<Point&>(msg);
            std::cout << "Point: x=" << p.x << ", y=" << p.y << std::endl;
        } else {
            std::cout << "others" << std::endl;
        }
        return 0;
    });
    actor->Send(1);
    actor->Send(2.0);
    actor->Send('3');

    std::future<ccl::any> future1;
    std::future<ccl::any> future2;
    {
        ccl::ActorSystem system;
        system.Register(actor, "/path/actor1");
        future1 = system.Send((char*) "4", "/path/actor1");
        future2 = system.Send((char*) "5", "/path/actor2");
        system.Broadcast(std::string("6"));
        system.Broadcast(Point{7, 8});
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
