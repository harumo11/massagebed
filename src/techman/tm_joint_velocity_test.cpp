//
//このプログラムは一番先端の軸が少しの角度だけ回転し，戻ってくるだけのものです．
//
#define _USE_MATH_DEFINES

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>

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

int main(void)
{

    //test for checksum()
    //int checksum_res = checksum("TMSCT,9,0,Listen1,");
    //std::cout << "[chechsum test] : " << std::hex << checksum_res << "\t=\t4c" << std::endl;

    //test for create_command()
    //auto created_command = create_command("Listen1");
    //std::cout << "[create_command test] : " << created_command << "\t=\t" << R"($TMSCT,9,0,Listen1,*4C\r\n)" << std::endl;
    //return 0;

    //setting
    Poco::Net::SocketAddress server_address("192.168.10.2", 5890);
    Poco::Net::StreamSocket socket;

    //connect to tm robot
    while (true) {
        try {
            std::cout << "[CONNECT] Now try to connect to TM" << std::endl;
            socket.connect(server_address);
        } catch (const Poco::Exception& exc) {
            std::cout << exc.displayText() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        break;
    }
    std::cout << "[OK] connected" << std::endl;

    //make tm robot set joint velocity mode
    std::string start_joint_vel_mode = create_command("ContinueVJog()");
    //std::string start_joint_vel_mode = create_command("Listen1");
    std::cout << "[Command] " << start_joint_vel_mode << std::endl;

    int byte = socket.sendBytes(start_joint_vel_mode.c_str(), start_joint_vel_mode.length());
    std::cout << "[Written] " << byte << std::endl;
    char received_data[256];
    byte = socket.receiveBytes(received_data, strlen(received_data));
    received_data[byte] = '\0';
    std::cout << "[Receive] " << received_data << std::endl;

    //send command which rotate frange joint with 1deg/s
    double numrator = 90;
    auto start_time_point1 = std::chrono::high_resolution_clock::now();
    int sleep_time = 50;
    bool is_display = true;
    for (double i = 0; i < 2 * M_PI; i += 2 * M_PI * 0.01) {
        std::stringstream velocity_command;
        velocity_command << "SetContinueVJog(0,0,0,0,0," << std::fixed << std::setprecision(3) << -0.5 * numrator * std::sin(i) << ")";
        std::string joint_velocity_command = create_command(velocity_command.str());
        socket.sendBytes(joint_velocity_command.c_str(), joint_velocity_command.length());
        char read_buff[256];
        //socket.receiveBytes(read_buff, sizeof(read_buff));
        //read_buff[byte] = '\0';
        //if (is_display)
        //{
        //       std::cout << "[Forward] " << i << std::endl;
        //       std::system("cls");
        //       std::cout << "[Send] " << joint_velocity_command << std::endl;
        //       std::cout << "[Read] " << read_buff << std::endl;
        //}
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }
    auto end_time_point1 = std::chrono::high_resolution_clock::now();

    //de-initialization
    std::string stop_joint_vel_mode = create_command("StopContinueVmode()");
    socket.sendBytes(stop_joint_vel_mode.c_str(), stop_joint_vel_mode.length());
    char read_buff[256];
    socket.receiveBytes(read_buff, sizeof(read_buff));
    read_buff[byte] = '\0';
    std::cout << "[Read] " << read_buff << std::endl;

    //measure elapsed time
    std::chrono::duration<double> elapsed_time1 = end_time_point1 - start_time_point1;
    std::cout << "[Time1] "
              << "Forward elapsed time " << elapsed_time1.count() << "\t Ideal time : 1sec" << std::endl;
}
