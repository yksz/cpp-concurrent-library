#include <future>
#include <iostream>
#include <typeinfo>
#include "ccl/actor.h"

namespace {

struct Point {
    int x;
    int y;
};

} // unnamed namespace

int main(void) {
    using namespace ccl;

    auto actor = std::make_shared<ccl::Actor>([](any&& msg) -> any {
        const std::type_info& type = msg.type();
        if (type == typeid(int)) {
            std::cout << "int: " << any_cast<int>(msg) << std::endl;
        } else if (type == typeid(double)) {
            std::cout << "double: " << any_cast<double>(msg) << std::endl;
        } else if (type == typeid(char)) {
            std::cout << "char: " << any_cast<char>(msg) << std::endl;
        } else if (type == typeid(char*)) {
            std::cout << "char*: " << any_cast<char*>(msg) << std::endl;
        } else if (type == typeid(std::string)) {
            std::cout << "string: " << any_cast<std::string>(msg) << std::endl;
        } else if (type == typeid(Point)) {
            Point& p = any_cast<Point&>(msg);
            std::cout << "Point: x=" << p.x << ", y=" << p.y << std::endl;
        } else {
            std::cout << "others" << std::endl;
            return std::string("others: ok");
        }
        return 0;
    });
    actor->Send(1);
    actor->Send(2.0);
    actor->Send('3');
    actor->Send((char*) "4");
    actor->Send(std::string("5"));
    actor->Send(Point{6, 7});
    auto future = actor->Send(8L);
    auto response = future.get();
    if (response.type() == typeid(std::string)) {
        std::cout << "response: " << any_cast<std::string>(response) << std::endl;
    }
    return 0;
}
