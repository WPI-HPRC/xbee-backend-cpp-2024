cmake_minimum_required(VERSION 3.24)
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
        XBee/XBeeDevice.cpp
        XBee/XBeeDevice.h
        Utility.h
        WebServer.cpp
        WebServer.h
        Backend.cpp
        Backend.h
        DataSimulator.cpp
        DataSimulator.h
        json_struct.h
        DataLogger.cpp
        DataLogger.h
        Constants.h
        XBee/XBeeUtility.h
        XBee/CircularBuffer.hpp
        RadioModule.cpp
        RadioModule.h
        SerialPort.cpp
        SerialPort.h
        XBee/circularQueue.hpp
        hprcWebSocket.cpp
        hprcWebSocket.h
        Secrets.h)

target_link_libraries(xbee_backend_cpp
        Qt::Core
        Qt6::SerialPort
        Qt6::WebSockets
)
