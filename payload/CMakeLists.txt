cmake_minimum_required(VERSION 3.29)
project(payload)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp
        senders/sender.h
        senders/telegram_sender.cpp
        senders/telegram_sender.h
        net.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES} openssl::openssl)
target_include_directories(${PROJECT_NAME} PRIVATE .)