cmake_minimum_required(VERSION 3.12)
project(integrate)
add_definitions("-std=c++2a")

add_executable(simple_force_reaction ./src/simple_force_reaction.cpp)
add_executable(constant_force_manager ./src/constant_force_manager.cpp)
add_executable(simple_force_massage ./src/simple_force_massage.cpp)

target_link_libraries(simple_force_reaction PocoFoundation PocoNet)
target_link_libraries(constant_force_manager PocoFoundation PocoNet)
target_link_libraries(simple_force_massage PocoFoundation PocoNet)

