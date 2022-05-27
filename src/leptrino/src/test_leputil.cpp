#include "../include/leputil.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    std::cout << "[Info] application starts." << std::endl;
    leptrino sensor;
    while (true) {
        auto measured_data = sensor.receive_data();
        for (auto&& e : measured_data) {
            std::cout << std::fixed << std::setprecision(3) << e << "\t";
        }
        std::cout << std::endl;
        std::cout << "-----------------------------------------------" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
