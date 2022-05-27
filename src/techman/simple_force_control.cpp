#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/StreamCopier.h>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <tuple>

int checksum(const std::string data, const bool istest = false)
{
    std::vector<int> separated_data;
    for (auto c : data) {
        if (istest) {
            std::cout << std::hex << "0x" << int(c) << std::endl;
            std::cout << std::dec << std::endl;
        }
        separated_data.push_back(int(c));
    }

    int checksum = 0;
    for (auto e : separated_data) {
        checksum ^= e;
    }

    return checksum;
}

std::string create_command(const std::string Script, bool istest = false)
{
    std::stringstream command;
    std::stringstream data_for_checksum;
    std::stringstream Data;

    Data << "0"
         << "," << Script;
    data_for_checksum << "TMSCT"
                      << "," << Data.str().size() << "," << Data.str() << ",";
    const int checksum_result = checksum(data_for_checksum.str());
    std::stringstream checksum_as_string;
    checksum_as_string << std::hex << checksum_result;
    command << "$" << data_for_checksum.str() << "*" << checksum_as_string.str() << "\r\n";
    if (istest) {
        std::cout << "[create_command] "
                  << "Data\t" << Data.str() << "\tData size\t" << Data.str().size() << std::endl;
        std::cout << "[Checksum] : " << std::hex << checksum_result;
        std::cout << std::dec << std::endl;
    }

    return command.str();
}

std::vector<double> split_message(const std::string message)
{
    std::cout << "[Debug] split_message() " << std::endl;
    std::cout << "[Debug] message : " << message << std::endl;
    // find DATA: and remove from msgs
    std::string header("DATA");
    std::cout << "[Debug] header size : " << header.size() << std::endl;
    std::cout << "[Debug] find first of header : " << message.find_first_of(header) << std::endl;
    std::string msg_header_removed = message.substr(message.find_first_of(header) + header.size());
    std::cout << "[Debug] msg_header_removed : " << msg_header_removed << std::endl;
    // find : and remove from msgs
    std::string msg_footer_removed = msg_header_removed.erase(msg_header_removed.find_first_of(":"));
    std::cout << "[Debug] msg_footer_removed : " << msg_footer_removed << std::endl;
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
    std::cout << "[Debug] remove_bias() " << std::endl;
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
    std::cout << "[Info] The application starts." << std::endl;
    Poco::Net::SocketAddress address_to_robot("192.168.10.2", 5890);
    Poco::Net::StreamSocket robot_socket;
    Poco::Net::SocketAddress address_to_leptrino(50010);
    Poco::Net::StreamSocket leptrino_socket;

    while (true) {
        try {
            std::cout << "[Info] Now trying to connect to leptrino." << std::endl;
            leptrino_socket.connect(address_to_leptrino);
        } catch (Poco::Exception& exc) {
            std::cout << exc.displayText() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        break;
    }
    std::cout << "[Info] leptrino is connected" << std::endl;

    while (true) {
        try {
            std::cout << "[Info] Now trying to connect to robot." << std::endl;
            robot_socket.connect(address_to_robot);
        } catch (Poco::Exception& exc) {
            std::cout << exc.displayText() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        break;
    }
    std::cout << "[Ok] robot is connected" << std::endl;

    // Measure and save biase
    std::cout << "[Info] The leptrino biases will be measure and saved" << std::endl;
    char leptorino_msgs[256];
    int leptorino_msgs_size = leptrino_socket.receiveBytes(leptorino_msgs, sizeof(leptorino_msgs));
    leptorino_msgs[leptorino_msgs_size] = '\0';
    std::string data(leptorino_msgs);
    auto biases = split_message(data);

    std::cout << "[Info] Techman cartecian mode will be set" << std::endl;
    std::string start_cartecian_vel_mode = create_command("ContinueVLine(1000, 1000)");
    std::cout << "[Info] " << start_cartecian_vel_mode << std::endl;
    robot_socket.sendBytes(start_cartecian_vel_mode.c_str(), start_cartecian_vel_mode.length());
    char received_data[256];
    int robot_msgs_size = robot_socket.receiveBytes(received_data, sizeof(received_data));
    received_data[robot_msgs_size] = '\0';
    std::cout << "[Info] "
              << "From robot : " << received_data << std::endl;

    int i = 0;
    while (true) {
        char message[256];
        int byte = leptrino_socket.receiveBytes(message, sizeof(message));
        std::cout << "[Debug] recieved byte size is " << byte << std::endl;
        message[byte] = '\0';
        std::string data(message);
        auto force_torque_vec = split_message(data);
        force_torque_vec = remove_bias(force_torque_vec, biases);
        std::cout << "[raw] "
                  << "[ " << i++ << " ] " << data << std::endl;
        std::cout << "[read] "
                  << "[ " << i << " ] " << force_torque_vec.at(0) << "\t" << force_torque_vec.at(1) << "\t" << force_torque_vec.at(2) << std::endl;
        if (std::abs(force_torque_vec.at(0)) > 3) {
            std::stringstream velocity_command;
            velocity_command << "SetContinueVLine(" << std::fixed << std::setprecision(3) << 0.1 * std::sin(i) << ",0,0,0,0,0)";
            std::string cartecian_velocity_command = create_command(velocity_command.str());
            robot_socket.sendBytes(cartecian_velocity_command.c_str(), cartecian_velocity_command.length());
        }
        std::cout << "------------------------" << std::endl;
    }

    std::string stop_cartecian_vel_mode = create_command("StopContinueVmode()");
    robot_socket.sendBytes(stop_cartecian_vel_mode.c_str(), stop_cartecian_vel_mode.length());
    char read_buff[256];
    robot_msgs_size = robot_socket.receiveBytes(read_buff, sizeof(read_buff));
    read_buff[robot_msgs_size] = '\0';
    std::cout << "[Read] " << read_buff << std::endl;

    return 0;
}
