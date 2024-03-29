//
// Created by William Scheirey on 3/20/24.
//

#ifndef XBEE_BACKEND_CPP_RADIOMODULE_H
#define XBEE_BACKEND_CPP_RADIOMODULE_H

#include "XBee/XBeeDevice.h"
#include "WebServer.h"
#include "SerialPort.h"
#include <boost/asio.hpp>

class RadioModule : public XBeeDevice
{
public:
    RadioModule();

    WebServer *webServer;

    boost::asio::io_service io_service;

    SerialPort *serialPort;

    void serialRead(uint8_t *buffer, size_t length_bytes) override;

    void serialWrite(const char *data, size_t length_bytes) override;

    void packetRead() override;

    void handleReceivePacket(XBee::ReceivePacket::Struct *frame) override;
};


#endif //XBEE_BACKEND_CPP_RADIOMODULE_H
