//このプログラムは一定の押し下げ力をロボットに発生させるものです．
//力覚センサからのデータをもとに力が一定になるように制御します．

#include "../../leptrino/include/leputil.hpp"
#include "../../techman/tmutil.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
    std::cout << "[Info] Application starts" << std::endl;
    leptrino sensor;
    techman robot;
    robot.set_motion_mode("cart");

    const double refference_force_z = 3;
    const double gain = -0.001;

    std::ofstream log("constant_force_manager_log.csv");
    log << "refference_force_z," << refference_force_z << ",gain," << gain << std::endl;
    log << "error, u" << std::endl;

    while (true) {
        //センサーから最新のデータ取得
        auto measured_force = sensor.receive_data();
        auto measured_force_z = -1 * measured_force.at(2);

        //偏差を計算
        double error = refference_force_z - measured_force_z;
        std::cout << "[Info] error : " << error << std::endl;

        //ゲインをかける
        double u = gain * error;
        std::cout << "[Info] u : " << u << std::endl;

        //記録
        log << error << "," << u << std::endl;

        //ロボットに送信
        std::vector<double> velocity_command = { 0, 0, 0, 0, 0, 0 };
        velocity_command.at(2) = u;
        robot.move(velocity_command);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
