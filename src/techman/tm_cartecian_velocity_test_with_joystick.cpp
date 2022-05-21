//
//このプログラムはPS4のコントローラDualShock4を使用してTMロボットアームの手先ｘ座標を移動させる物です．
//
#define _USE_MATH_DEFINES

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cmath>

#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>

#include "JoyShockLibrary.h"

int checksum(const std::string data, const bool istest = false) {
        std::vector<int> separated_data;
        for (auto c : data) {
                if (istest)
                {
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

std::string create_command(const std::string Script, bool istest = false) {
        std::stringstream command;
        std::stringstream data_for_checksum;
        std::stringstream Data;

        Data << "0" << "," << Script;
        data_for_checksum << "TMSCT" << "," << Data.str().size() << "," << Data.str() << ",";
        const int checksum_result = checksum(data_for_checksum.str());
        std::stringstream checksum_as_string;
        checksum_as_string << std::hex << checksum_result;
        command << "$" << data_for_checksum.str() << "*" << checksum_as_string.str() << "\r\n";
        if (istest)
        {
                std::cout << "[create_command] " << "Data\t" << Data.str() << "\tData size\t" << Data.str().size() << std::endl;
                std::cout << "[Checksum] : " << std::hex << checksum_result;
                std::cout << std::dec << std::endl;
        }

        return command.str();
}

int main(void) {

        //setting
        //techman
        Poco::Net::SocketAddress server_address("192.168.10.2", 5890);
        Poco::Net::StreamSocket socket;
        //joystick
        if (JslConnectDevices() == 0)
        {
                std::cout << "[ERROR] No devices are connected. Exit" << std::endl;
                return 0;
        }
        int device_handlers[3];
        JslGetConnectedDeviceHandles(device_handlers, JslConnectDevices());

        //connect to tm robot
        while (true)
        {
                try
                {
                        std::cout << "[CONNECT] Now try to connect to TM" << std::endl;
                        socket.connect(server_address);
                }
                catch (const Poco::Exception& exc)
                {
                        std::cout << exc.displayText() << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        continue;
                }
                break;
        }
        std::cout << "[OK] connected" << std::endl;

        //make tm robot set joint velocity mode
        std::string start_cartecian_vel_mode = create_command("ContinueVLine(1000, 1000)");
        std::cout << "[Command] " << start_cartecian_vel_mode << std::endl;

        int byte = socket.sendBytes(start_cartecian_vel_mode.c_str(), start_cartecian_vel_mode.length());
        std::cout << "[Written] " << byte << std::endl;
        char received_data[256];
        byte = socket.receiveBytes(received_data, strlen(received_data));
        received_data[byte] = '\0';
        std::cout << "[Receive] " << received_data << std::endl;

        //send command which rotate frange joint with 1deg/s
        const double scale_factor = 0.05;
        int sleep_time = 10;
        bool is_display = false;
        while(true)
        {
                //get current joystick state
                auto joy_state = JslGetSimpleState(device_handlers[0]);

                //send command
                std::stringstream velocity_command;
                //for avoiding the bug std::setprecision(2) is used. 
                velocity_command << "SetContinueVLine(" << std::fixed << std::setprecision(2) << scale_factor * joy_state.stickRX << ",0,0,0,0,0)";
                std::string cartecian_velocity_command = create_command(velocity_command.str());
                std::cout << "[Send]\t" << cartecian_velocity_command << std::endl;
                socket.sendBytes(cartecian_velocity_command.c_str(), cartecian_velocity_command.length());
                char read_buff[256];
                if (is_display)
                {
                        //read make the communication too slow
                        socket.receiveBytes(read_buff, sizeof(read_buff));
                        read_buff[byte] = '\0';
                        std::system("cls");
                        std::cout << "[Send] " << cartecian_velocity_command << std::endl;
                        std::cout << "[Read] " << read_buff << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }

        //de-initialization
        std::string stop_cartecian_vel_mode = create_command("StopContinueVmode()");
        socket.sendBytes(stop_cartecian_vel_mode.c_str(), stop_cartecian_vel_mode.length());
        char read_buff[256];
        socket.receiveBytes(read_buff, sizeof(read_buff));
        read_buff[byte] = '\0';
        std::cout << "[Read] " << read_buff << std::endl;
}
