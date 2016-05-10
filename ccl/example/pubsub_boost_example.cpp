#include <iostream>
#include <typeinfo>
#define CCL_ANY_USING_BOOST
#include "ccl/pubsub.h"

int main(void) {
    using namespace boost;

    std::string recvMsg1;
    std::string recvMsg2;
    std::string recvMsg3;
    std::string recvMsg4;

    auto actor1 = std::make_shared<ccl::Actor>([&](any&& msg) -> any {
        const std::type_info& type = msg.type();
        if (type == typeid(std::string)) {
            recvMsg1 += any_cast<std::string>(msg);
        }
        return 0;
    });
    auto actor2 = std::make_shared<ccl::Actor>([&](any&& msg) -> any {
        const std::type_info& type = msg.type();
        if (type == typeid(std::string)) {
            recvMsg2 += any_cast<std::string>(msg);
        }
        return 0;
    });
    auto actor3 = std::make_shared<ccl::Actor>([&](any&& msg) -> any {
        const std::type_info& type = msg.type();
        if (type == typeid(std::string)) {
            recvMsg3 += any_cast<std::string>(msg);
        }
        return 0;
    });
    auto actor4 = std::make_shared<ccl::Actor>([&](any&& msg) -> any {
        const std::type_info& type = msg.type();
        if (type == typeid(std::string)) {
            recvMsg4 += any_cast<std::string>(msg);
        }
        return 0;
    });

    ccl::PubSub broker;
    broker.Subscribe("/a/1", {actor1, actor2});
    broker.Subscribe("/a/2", actor3);
    broker.Subscribe("/b/2", actor4);
    broker.Publish("/a/1", std::string(" publish"));
    broker.Multicast("/a/#", std::string(" multicast#"));
    broker.Multicast("/+/2", std::string(" multicast+"));
    broker.Broadcast(std::string(" broadcast"));

    std::cout << "actor1:" << recvMsg1 << std::endl;
    std::cout << "actor2:" << recvMsg2 << std::endl;
    std::cout << "actor3:" << recvMsg3 << std::endl;
    std::cout << "actor4:" << recvMsg4 << std::endl;
    return 0;
}