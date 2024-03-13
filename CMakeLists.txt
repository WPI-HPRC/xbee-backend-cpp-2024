cmake_minimum_required(VERSION 3.27)
project(xbee_backend_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        SerialPort
        WebSockets
        REQUIRED)

add_executable(xbee_backend_cpp main.cpp
        SerialReader.cpp
        SerialReader.h
        XBeeDevice.cpp
        XBeeDevice.h
        Utility.h
        WebServer.cpp
        WebServer.h
        Backend.cpp
        Backend.h)
target_link_libraries(xbee_backend_cpp
        Qt::Core
        Qt6::SerialPort
        Qt6::WebSockets
)

