cmake_minimum_required(VERSION 3.12)
project(integrate)
add_definitions("-std=c++2a")

add_executable(simple_force_reaction ./src/simple_force_reaction.cpp)
target_link_libraries(simple_force_reaction PocoFoundation PocoNet)

