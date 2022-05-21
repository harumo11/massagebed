#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/StreamCopier.h>
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    //Poco::Net::SocketAddress address_to_server("127.0.0.1", 8080);
    Poco::Net::SocketAddress address_to_server(50010);
    Poco::Net::StreamSocket ss;

    while (true) {
        try {
            ss.connect(address_to_server);
        } catch (Poco::Exception& exc) {
            std::cout << exc.displayText() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        break;
    }

    //message_stream << "hello" << std::endl;
    std::cout << "[Ok] connect" << std::endl;
    int i = 0;
    while (true) {
        //if (ss.available() > 0) {
        char message[256];
        int byte = ss.receiveBytes(message, sizeof(message));
        message[byte] = '\0';
        std::cout << "[read] "
                  << "[ " << i++ << " ] " << message << "\t" << byte << std::endl;
        std::cout << "------------------------" << std::endl;
        //}
    }

    return 0;
}
