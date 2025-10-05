cmake_minimum_required(VERSION 3.16)

project(SnapsSandbox)

add_executable(SnapsSandbox Main.cpp)
target_link_libraries(SnapsSandbox PRIVATE Snaps raylib)
