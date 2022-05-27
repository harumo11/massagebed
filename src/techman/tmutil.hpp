#pragma once

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

class techman {

public:
    techman();
    int checksum(const std::string message, const bool verbose = false);
    std::string create_command(const std::string script, const bool verbose = false);
    void try_connect(const std::string ipaddress = "192.168.10.2", const unsigned int port = 5890);
    void try_disconnect();
    int send_script(const std::string script);
    std::string recv_script();
    bool move(const std::vector<double> velocity_command);
    bool set_motion_mode(const std::string mode = "cart");
    Poco::Net::StreamSocket socket_to_robot;
    std::string current_mode;
};

techman::techman()
{
}

void techman::try_connect(const std::string ipaddress, const unsigned int port)
{
    Poco::Net::SocketAddress techman_address(ipaddress, port);

    while (true) {
        try {
            std::cout << "[Info] Now trying to connect to robot." << std::endl;
            this->socket_to_robot.connect(techman_address);
        } catch (Poco::Exception& exc) {
            std::cout << exc.displayText() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        break;
    }
    std::cout << "[Info] Techman robot is connected" << std::endl;
}

void techman::try_disconnect()
{
    this->socket_to_robot.shutdown();
    this->socket_to_robot.close();
}

int techman::send_script(const std::string script)
{
    int script_size = this->socket_to_robot.sendBytes(script.c_str(), script.size());
    return script_size;
}

std::string techman::recv_script()
{
    char received_buffer[4096];
    int received_message_size = this->socket_to_robot.receiveBytes(received_buffer, sizeof(received_buffer));
    return std::string(received_buffer, received_message_size);
}

/**
 * @brief This cunction calculate the checksum of message.
 *
 * @param message The command of the techman.
 * @param verbose If true, print internal calculate and state.
 *
 * @return calculated chacksum.
 */
int techman::checksum(const std::string message, const bool verbose)
{
    std::vector<int> separated_data;
    for (auto c : message) {
        if (verbose) {
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

/**
 * @brief This function adds the header, footer and, checksum to script.
 *
 * @param script The techman command such as a joint velocity command.
 * @param verbose If true, print internal calculate and state.
 *
 * @return The complete message that is sendable.
 */
std::string techman::create_command(const std::string script, bool verbose)
{
    std::stringstream command;
    std::stringstream data_for_checksum;
    std::stringstream Data;

    Data << "0"
         << "," << script;
    data_for_checksum << "TMSCT"
                      << "," << Data.str().size() << "," << Data.str() << ",";
    const int checksum_result = this->checksum(data_for_checksum.str());
    std::stringstream checksum_as_string;
    checksum_as_string << std::hex << checksum_result;
    command << "$" << data_for_checksum.str() << "*" << checksum_as_string.str()
            << "\r\n";
    if (verbose) {
        std::cout << "[create_command] "
                  << "Data\t" << Data.str() << "\tData size\t" << Data.str().size()
                  << std::endl;
        std::cout << "[Checksum] : " << std::hex << checksum_result;
        std::cout << std::dec << std::endl;
    }

    return command.str();
}

bool techman::set_motion_mode(const std::string mode)
{
    this->current_mode = mode;

    if (mode == "joint") {
        std::string stop_mode_command = this->create_command("StopContinueVLine()");
        int send_script_size = this->send_script(stop_mode_command);
        std::cout << "[Debug] send script size : " << send_script_size << std::endl;

        std::string joint_mode_command = this->create_command("ContinueVJog()");
        send_script_size = this->send_script(joint_mode_command);
        std::cout << "[Debug] send script size : " << send_script_size << std::endl;

        std::string received_script = this->recv_script();
        std::cout << "[Debug] received script : " << received_script << std::endl;
        return true;
    } else { // mode == "cart"
        std::string stop_mode_command = this->create_command("StopContinueVJog()");
        int send_script_size = this->send_script(stop_mode_command);
        std::cout << "[Debug] send script size : " << send_script_size << std::endl;

        std::string cart_mode_command = this->create_command("ContinueVLine(1000, 1000)");
        send_script_size = this->send_script(cart_mode_command);
        std::cout << "[Debug] send script size : " << send_script_size << std::endl;

        const std::string received_script = this->recv_script();
        std::cout << "[Debug] received script : " << received_script << std::endl;
        return true;
    }
}

bool techman::move(const std::vector<double> velocity_command)
{
    // check wheterh the command has six value.
    if (velocity_command.size() != 6) {
        std::cout << "[Warn] The velocity command size must be six. But your velocity command size is : " << velocity_command.size() << " The command will not send." << std::endl;
        return false;
    }

    // create techman script
    std::stringstream velocity_data;
    if (this->current_mode == "joint") {
        velocity_data << "SetContinueVJog(" << std::fixed << std::setprecision(3)
                      << velocity_command.at(0) << "," << velocity_command.at(1) << "," << velocity_command.at(2) << "," << velocity_command.at(3) << "," << velocity_command.at(4) << "," << velocity_command.at(5) << ")";
    } else { //current_mode == cart
        velocity_data << "SetContinueVLine(" << std::fixed << std::setprecision(3)
                      << velocity_command.at(0) << "," << velocity_command.at(1) << "," << velocity_command.at(2) << "," << velocity_command.at(3) << "," << velocity_command.at(4) << "," << velocity_command.at(5) << ")";
    }

    //send the script and receive result
    std::string velocity_script = this->create_command(velocity_data.str());
    this->send_script(velocity_script);
    std::cout << "[Debug] " << this->recv_script() << std::endl;

    return true;
}
