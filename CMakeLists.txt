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
        xbee/XBeeDevice.cpp
        xbee/XBeeDevice.h
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
        xbee/XBeeUtility.h
        xbee/CircularBuffer.hpp
        RadioModule.cpp
        RadioModule.h
        SerialPort.cpp
        SerialPort.h
        xbee/circularQueue.hpp)

target_link_libraries(xbee_backend_cpp
        Qt::Core
        Qt6::SerialPort
        Qt6::WebSockets
        ${Boost_LIBRARIES}
)
