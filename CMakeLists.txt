cmake_minimum_required(VERSION 3.29)
project(fox_stealer)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system json circular_buffer intrusive smart_ptr)
find_package(OpenSSL REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(cryptopp REQUIRED)
find_package(libzippp REQUIRED)

add_subdirectory(payload)

