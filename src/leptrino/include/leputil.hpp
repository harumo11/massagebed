#pragma once
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/StreamCopier.h>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <chrono>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

class leptrino {

public:
    leptrino();
    void try_connect(const std::string ipaddress = "127.0.0.1", const unsigned int port = 50010);
    std::optional<std::vector<double>> split_message(const std::string message);
    std::vector<double> remove_bias(const std::vector<double> sensor_data,
        const std::vector<double> biases);
    void try_close();
    std::vector<double> reveive_data();
    std::vector<double> measure_bias();
    std::vector<double> biases = { 0, 0, 0, 0, 0, 0 };
    double mean(const std::vector<double>& sensor_data);
    Poco::Net::StreamSocket socket_to_leptrino;
    std::vector<double> last_unbiased_data = { 0, 0, 0, 0, 0, 0 };
};

leptrino::leptrino()
{
    this->try_connect();
    this->biases = this->measure_bias();
}

/**
 * @brief This function makes a connection to leptrino server and waits until that connection establishes.
 *
 * @param ipaddress The IP address of the PC that leptrino server is running. The default IP address is 127.0.0.1 that means the server and this program will run on same PC.
 * @param port The port of the leptrino server program. The default port is 50010.
 *
 * @return If connect to leptrino server successfully, return true.
 */
void leptrino::try_connect(const std::string ipaddress, const unsigned int port)
{
    Poco::Net::SocketAddress leptrino_server_addres(ipaddress, port);
    while (true) {
        try {
            this->socket_to_leptrino.connect(leptrino_server_addres);
        } catch (Poco::Exception& exc) {
            std::cout << "[Warn] " << exc.displayText() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << "[Info] "
                  << "The connection to leptrino is established successfully." << std::endl;
        break;
    }
}

/**
 * @brief This function converts from the message words to understandable sensor value.
 *
 * @param message The received message from leptrino server.
 *
 * @return The accutual sensor data.
 */
std::optional<std::vector<double>> leptrino::split_message(const std::string message)
{
    const std::string header = "DATA";
    //std::cout << "[Info] "
    //          << "recieved massage : " << message << std::endl;
    // find DATA: and remove from msgs
    std::string msg_header_removed = message.substr(message.find_first_of(header) + header.size());
    //std::cout << "[Info] "
    //          << "removed header massage : " << msg_header_removed << std::endl;
    // find : and remove from msgs
    std::string msg_footer_removed = msg_header_removed.erase(msg_header_removed.find_first_of(":"));
    //std::cout << "[Info] "
    //          << "removed footer massage : " << msg_footer_removed << std::endl;
    // sepalete msgs with ,
    Poco::StringTokenizer tokernizer(msg_footer_removed, ",");
    std::vector<double> force_torque_data;
    for (auto e : tokernizer) {
        force_torque_data.push_back(std::stod(e));
    }

    if (force_torque_data.size() != 6) {
        std::cout << "[Warn] Sensor data size is not 6. Some data dropped." << std::endl;
        return std::nullopt;
    } else {
        return force_torque_data;
    }
}

/**
 * @brief The leprino sensor data includes bias. This function removes it.
 *
 * @param sensor_data The sensor data that is splited each value.
 * @param biases The six values that is difference between true force and torque and the measured data.
 *
 * @return The six values that removed biases.
 */
std::vector<double> leptrino::remove_bias(const std::vector<double> sensor_data,
    const std::vector<double> biases)

{

    if (sensor_data.size() != 6 || biases.size() != 6) {
        std::cout << "[Warn] The size of unbiased data or bias data is not six. " << std::endl;
    }

    std::vector<double> unbiased_message;
    for (int i = 0; i < 6; i++) {
        unbiased_message.push_back(sensor_data.at(i) - biases.at(i));
    }

    return unbiased_message;
}

/**
 * @brief This function close the connection to leptrino server.
 *
 * @return If the connection close successfully, return True.
 */
void leptrino::try_close()
{
    this->socket_to_leptrino.shutdown();
    this->socket_to_leptrino.close();
    std::cout << "[Info] "
              << "The connection to leptrino is closed successfully." << std::endl;
}

/**
 * @brief This function receives the data and do pre-process such as removing bias.
 *
 * @return The force and toruqe sensor data. You should use this values to your calculations.
 */
std::vector<double> leptrino::reveive_data()
{
    char message[4096];
    int received_size = this->socket_to_leptrino.receiveBytes(message, sizeof(message));
    message[received_size] = '\0';
    std::string received_data(message);
    auto can_message_split = split_message(received_data);
    if (can_message_split) {
        this->last_unbiased_data = remove_bias(can_message_split.value(), this->biases);
        return this->last_unbiased_data;
    } else {
        std::cout << "[Warn] "
                  << "last unbiased data is used" << std::endl;
        return this->last_unbiased_data;
    }
}

/**
 * @brief This function measures data in 1 sec and returns that average values as the biases.
 *
 * @return Six bias.
 */
std::vector<double> leptrino::measure_bias()
{
    auto begintime = std::chrono::steady_clock::now();
    std::vector<double> bias_x;
    std::vector<double> bias_y;
    std::vector<double> bias_z;
    std::vector<double> bias_r;
    std::vector<double> bias_p;
    std::vector<double> bias_w;

    while (true) {
        char message[4096];
        int received_size = this->socket_to_leptrino.receiveBytes(message, sizeof(message));
        message[received_size] = '\0';
        std::string received_data(message);
        auto can_message_split = split_message(received_data);
        if (can_message_split) {
            bias_x.push_back(can_message_split.value().at(0));
            bias_y.push_back(can_message_split.value().at(1));
            bias_z.push_back(can_message_split.value().at(2));
            bias_r.push_back(can_message_split.value().at(3));
            bias_p.push_back(can_message_split.value().at(4));
            bias_w.push_back(can_message_split.value().at(5));
        }

        auto currenttime = std::chrono::steady_clock::now();
        auto diffsec = std::chrono::duration<double>(currenttime - begintime).count();
        if (diffsec > 1.0) {
            break;
        }
    }

    std::cout << "[Info] "
              << "The bias data is measured" << std::endl;
    std::vector<double> mean_biases = { this->mean(bias_x), this->mean(bias_y), this->mean(bias_z), this->mean(bias_r), this->mean(bias_p), this->mean(bias_w) };
    std::cout << "[Info] "
              << "The bias : x = " << mean_biases.at(0) << "\t y = " << mean_biases.at(1) << "\t z = " << mean_biases.at(2) << "\t roll = " << mean_biases.at(3) << "\t pitch = " << mean_biases.at(4) << "\t yaw = " << mean_biases.at(5) << std::endl;
    return mean_biases;
}

double leptrino::mean(const std::vector<double>& sensor_data)
{
    double sum = 0;
    for (auto&& value : sensor_data) {
        sum += value;
    }

    double mean = sum / sensor_data.size();
    return mean;
}
