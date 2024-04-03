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

    void readBytes(uint8_t *buffer, size_t length_bytes) override;

    void writeBytes(const char *data, size_t length_bytes) override;

    void packetRead() override;

    void handleReceivePacket(XBee::ReceivePacket::Struct *frame) override;

    void handleReceivePacket64Bit(XBee::ReceivePacket64Bit::Struct *frame) override;

    void start() override;

    void incorrectChecksum(uint8_t calculated, uint8_t received) override;

};


#endif //XBEE_BACKEND_CPP_RADIOMODULE_H
