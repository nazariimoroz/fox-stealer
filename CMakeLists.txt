cmake_minimum_required(VERSION 3.29)
project(fox_stealer)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system json)
find_package(OpenSSL REQUIRED)

add_subdirectory(payload)

