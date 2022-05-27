#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/StreamCopier.h>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <tuple>

std::vector<double> split_message(const std::string message)
{
    const std::string header = "DATA";
    std::cout << "[Info] "
              << "recieved massage : " << message << std::endl;
    // find DATA: and remove from msgs
    std::string msg_header_removed = message.substr(message.find_first_of(header) + header.size());
    std::cout << "[Info] "
              << "removed header massage : " << msg_header_removed << std::endl;
    // find : and remove from msgs
    std::string msg_footer_removed = msg_header_removed.erase(msg_header_removed.find_first_of(":"));
    std::cout << "[Info] "
              << "removed footer massage : " << msg_footer_removed << std::endl;
    // sepalete msgs with ,
    Poco::StringTokenizer tokernizer(msg_footer_removed, ",");
    std::vector<double> force_torque_data;
    for (auto e : tokernizer) {
        force_torque_data.push_back(std::stod(e));
    }

    if (force_torque_data.size() != 6) {
        std::cout << "[Warn] Sensor data size is not 6. Some data dropped." << std::endl;
    }
    return force_torque_data;
}

std::vector<double> remove_bias(const std::vector<double> message, const std::vector<double> biases)
{
    if (message.size() != 6 || biases.size() != 6) {
        std::cout << "[Warn] The size of unbiased data or bias data is not six. " << std::endl;
    }

    std::vector<double> unbiased_message;
    for (int i = 0; i < 6; i++) {
        unbiased_message.push_back(message.at(i) - biases.at(i));
    }

    return unbiased_message;
}

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

    std::cout << "[Ok] connect" << std::endl;

    // Measure and save biase
    char message[256];
    int byte = ss.receiveBytes(message, sizeof(message));
    std::string data(message);
    auto biases = split_message(data);

    int i = 0;
    while (true) {
        //if (ss.available() > 0) {
        char message[256];
        int byte = ss.receiveBytes(message, sizeof(message));
        message[byte] = '\0';
        std::string data(message);
        auto force_torque_vec = split_message(data);
        force_torque_vec = remove_bias(force_torque_vec, biases);
        std::cout << "[raw] "
                  << "[ " << i++ << " ] " << data << std::endl;
        std::cout << "[read] "
                  << "[ " << i << " ] " << force_torque_vec.at(0) << "\t" << force_torque_vec.at(1) << "\t" << force_torque_vec.at(2) << std::endl;
        std::cout << "------------------------" << std::endl;
        //}
    }

    return 0;
}
