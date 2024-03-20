cmake_minimum_required(VERSION 3.27)
project(xbee_backend_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(BOOST_ROOT /usr/local/Cellar/boost/1.80.0)
set(BOOST_INCLUDEDIR /usr/local/Cellar/boost/1.80.0/include)
set(BOOST_LIBRARY_DIR /usr/local/Cellar/boost/1.80.0/lib)

find_package(Boost 1.80.0 COMPONENTS system filesystem REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif ()


find_package(Qt6 COMPONENTS
        Core
        SerialPort
        WebSockets
        REQUIRED)

add_executable(xbee_backend_cpp main.cpp
        SerialReader.cpp
        SerialReader.h
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
        CircularBuffer.hpp
        RadioModule.cpp
        RadioModule.h
        SerialPort.cpp
        SerialPort.h)
target_link_libraries(xbee_backend_cpp
        Qt::Core
        Qt6::SerialPort
        Qt6::WebSockets
)

