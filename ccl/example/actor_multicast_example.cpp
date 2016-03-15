#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <typeinfo>
#include <vector>
#define CCL_ACTOR_DEBUG
#include "ccl/actor.h"

int main(void) {
    auto actor1 = std::make_shared<ccl::Actor>([](ccl::any&& msg) -> ccl::any {
        const std::type_info& type = msg.type();
        if (type == typeid(int)) {
            std::cout << "actor1: " << ccl::any_cast<int>(msg) << std::endl;
        }
        return 0;
    });
    auto actor2 = std::make_shared<ccl::Actor>([](ccl::any&& msg) -> ccl::any {
        const std::type_info& type = msg.type();
        if (type == typeid(int)) {
            std::cout << "actor2: " << ccl::any_cast<int>(msg) << std::endl;
        }
        return 0;
    });
    auto actor3 = std::make_shared<ccl::Actor>([](ccl::any&& msg) -> ccl::any {
        const std::type_info& type = msg.type();
        if (type == typeid(int)) {
            std::cout << "actor3: " << ccl::any_cast<int>(msg) << std::endl;
        }
        return 0;
    });

    ccl::ActorSystem system;
    system.Register(actor1, "/a/actor/1");
    system.Register(actor2, "/a/actor/2");
    system.Register(actor2, "/b/actor/2");
    system.Register(actor3, "/b/actor/3");
    system.Multicast(1, "/a/actor/#");
    system.Multicast(2, "/+/actor/2");
    return 0;
}
