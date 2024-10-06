cmake_minimum_required(VERSION 3.29)
project(fox_stealer)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system json)
find_package(OpenSSL REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(cryptopp REQUIRED)
find_package(libzippp REQUIRED)
find_package(Qt6 COMPONENTS Quick Gui Core QuickControls2 REQUIRED)

add_subdirectory(payload)
add_subdirectory(builder)

