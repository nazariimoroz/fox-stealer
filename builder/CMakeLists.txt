cmake_minimum_required(VERSION 3.29)
project(builder)
set(PROJECT_NAME_QT Builder)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_add_executable(${PROJECT_NAME}
        main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
        URI ${PROJECT_NAME_QT}
        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME_QT}"
        RESOURCE_PREFIX /
        VERSION 1.0
        QML_FILES
        view/Main.qml
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Quick
        Qt6::Gui
        Qt6::Core
        Qt6::QuickControls2)
target_include_directories(${PROJECT_NAME} PRIVATE
        .
        ./view/)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

# copying payload.exe
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${payload_BINARY_DIR}/payload.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS)
