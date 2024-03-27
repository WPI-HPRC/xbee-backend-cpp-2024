//
// Created by William Scheirey on 2/28/24.
//

#ifndef XBEE_BACKEND_CPP_SERIALREADER_H
#define XBEE_BACKEND_CPP_SERIALREADER_H

#include <boost/asio.hpp>
#include "XBee/XBeeDevice.h"
#include "RadioModule.h"
#include <thread>

class SerialReader
{

public:
    SerialReader();

    static RadioModule *radioModule;

//    boost::asio::io_service io_service;
    boost::asio::io_context io_context;

    std::thread thread_;
};


#endif //XBEE_BACKEND_CPP_SERIALREADER_H
