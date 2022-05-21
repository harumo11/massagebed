cmake_minimum_required(VERSION 3.12)
project(leptrino_force_torque)

add_executable(${PROJECT_NAME} src/leptrino_force_torque.cpp src/rs_comm.cpp)
add_executable(leptrino_test_client src/tcp_client.cpp)
add_executable(leptrino_test_client2 src/tcp_client2.cpp)

target_link_libraries(${PROJECT_NAME} PocoFoundation PocoNet)
target_link_libraries(leptrino_test_client PocoFoundation PocoNet)
target_link_libraries(leptrino_test_client2 PocoFoundation PocoNet)

