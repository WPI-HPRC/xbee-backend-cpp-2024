//
// Created by William Scheirey on 3/20/24.
//

#ifndef XBEE_BACKEND_CPP_RADIOMODULE_H
#define XBEE_BACKEND_CPP_RADIOMODULE_H

#include "XBee/XBeeDevice.h"
#include "SerialPort.h"
#include <boost/asio.hpp>

class RadioModule : public XBeeDevice
{
public:
    RadioModule();

    boost::asio::io_service io_service;

    void loop(const boost::system::error_code &error);

    void startLoop();

    SerialPort serialPort;

    void serialRead(const char *buffer, size_t length_bytes) override;

    void serialWrite(const char *data, size_t length_bytes) override;
};


#endif //XBEE_BACKEND_CPP_RADIOMODULE_H
