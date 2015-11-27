#include <iostream>
#include "ccl/actor.h"

int main(void) {
    auto actor = std::make_shared<ccl::Actor>([](const std::string& message) {
        std::cout << message;
    });
    actor->Tell("1");
    actor->Tell("2");
    actor->Tell("3");

    ccl::ActorSystem& system = ccl::ActorSystem::GetInstance();
    system.Register("/path/actor1", actor);
    system.Send("/path/actor1", "4");
    system.Send("/path/actor2", "5");
    system.Broadcast("\n");
    return 0;
}
