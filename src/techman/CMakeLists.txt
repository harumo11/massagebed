cmake_minimum_required(VERSION 3.12)
project(techman)
add_definitions("-std=c++2a")

add_executable(tm_joint_velocity tm_joint_velocity_test.cpp)
add_executable(tm_cartecian_velocity tm_cartecian_velocity_test.cpp)
#add_executable(simple_force_control simple_force_control.cpp)
add_executable(test_tmutil test_tmutil.cpp)

target_link_libraries(tm_joint_velocity PocoFoundation PocoNet)
target_link_libraries(tm_cartecian_velocity PocoFoundation PocoNet)
#target_link_libraries(simple_force_control PocoFoundation PocoNet)
target_link_libraries(test_tmutil PocoFoundation PocoNet)
