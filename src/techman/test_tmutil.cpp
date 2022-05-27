#include "tmutil.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
    techman robot;
    robot.set_motion_mode("joint");
    std::vector<double> velocity = { 0, 0, 0.1, 0, 0, 0 };

    for (int i = 0; i < 3000; i++) {
        robot.move(velocity);
        std::cout << "[Debug] send!" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "[Debug] See you!" << std::endl;

    return 0;
}
