#include "../../leptrino/include/leputil.hpp"
#include "../../techman/tmutil.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    std::cout << "[Info] Application starts" << std::endl;
    leptrino sensor;
    techman robot;
    robot.set_motion_mode();

    std::cout << "[Info] Control loop starts" << std::endl;
    const double force_threshold = 10.0;

    std::vector<double> move_velocity = { 0, 0, 0.05, 0, 0, 0 };
    std::vector<double> stop_velocity = { 0, 0, 0, 0, 0, 0 };

    while (true) {
        auto force_torque_values = sensor.receive_data();

        if (-1 * force_torque_values.at(2) > force_threshold) {
            std::cout << "[Info] move" << std::endl;
            robot.move(move_velocity);
        } else {
            std::cout << "[Info] stop" << std::endl;
            robot.move(stop_velocity);
        }
    }
    return 0;
}
