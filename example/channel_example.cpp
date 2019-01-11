#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include "ccl/channel.h"

int main(void) {
    const std::string sendMsg1 = "message1";
    const std::string sendMsg2 = "message2";

    ccl::Channel<std::string> chan;
    std::thread th([&]() {
        std::cout << "sending...: " << sendMsg1 << std::endl;
        chan.Send(sendMsg1);
        std::cout << "sending...: " << sendMsg2 << std::endl;
        chan << sendMsg2;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "start receiving" << std::endl;
    std::string recvMsg1 = chan.Receive();
    std::cout << "received: " << recvMsg1 << std::endl;
    std::string recvMsg2;
    chan >> recvMsg2;
    std::cout << "received: " << recvMsg2 << std::endl;

    th.join();

    // Output:
    // sending...: message1
    // start receiving
    // received: message1
    // sending...: message2
    // received: message2
    return 0;
}
