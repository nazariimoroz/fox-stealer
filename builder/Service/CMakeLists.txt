
project(Service LANGUAGES CXX)

set(${PROJECT_NAME}_SRC
        PayloadGeneratingService.cpp
        PayloadGeneratingService.h
)

qt_add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRC})
qt6_add_qml_module(${PROJECT_NAME}
        URI ${PROJECT_NAME}
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml"
        SOURCES
        ${${PROJECT_NAME}_SRC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml)
