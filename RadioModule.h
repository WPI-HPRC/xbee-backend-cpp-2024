//
// Created by William Scheirey on 3/20/24.
//

#ifndef XBEE_BACKEND_CPP_RADIOMODULE_H
#define XBEE_BACKEND_CPP_RADIOMODULE_H

#include "XBee/XBeeDevice.h"
#include "WebServer.h"
#include "SerialPort.h"
#include "hprcWebSocket.h"
#include <QTimer>

class RadioModule : public XBeeDevice
{
public:
    RadioModule();

    WebServer *webServer;

    SerialPort *serialPort;

    void readBytes(uint8_t *buffer, size_t length_bytes) override;

    void writeBytes(const char *data, size_t length_bytes) override;

    void packetRead() override;

    void handleReceivePacket(XBee::ReceivePacket::Struct *frame) override;

    void handleReceivePacket64Bit(XBee::ReceivePacket64Bit::Struct *frame) override;

    void start() override;

    void incorrectChecksum(uint8_t calculated, uint8_t received) override;

    void
    _handleRemoteAtCommandResponse(const uint8_t *frame, uint8_t length_bytes, bool paramWasBeingWaitedOn) override;

    void log(const char *format, ...) override;

    void didCycle() override;

    unsigned int cycleCount = 0;

    uint64_t avBayAddr = 0x0013a200422cdf59;

    RocketTelemPacket dummyPacket;
};


#endif //XBEE_BACKEND_CPP_RADIOMODULE_H
