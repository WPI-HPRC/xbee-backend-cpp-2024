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
    RadioModule(int baudRate, DataLogger *logger, const QSerialPortInfo &portInfo);

    RadioModule(int baudRate, DataLogger *logger);

    DataLogger *dataLogger;

    SerialPort *serialPort;

    QString name;

    DataLogger::Packet lastPacket;

    unsigned int cycleCountsFromFrameID[255];

    void configureRadio();

    void readBytes(uint8_t *buffer, size_t length_bytes) override;

    void writeBytes(const char *data, size_t length_bytes) override;

    void packetRead() override;

    void handleReceivePacket(XBee::ReceivePacket::Struct *frame) override;

    void handleReceivePacket64Bit(XBee::ReceivePacket64Bit::Struct *frame) override;

    void start() override;

    void incorrectChecksum(uint8_t calculated, uint8_t received) override;

    void
    _handleRemoteAtCommandResponse(const uint8_t *frame, uint8_t length_bytes) override;

    void handleExtendedTransmitStatus(const uint8_t *frame, uint8_t length_bytes) override;

    void sentFrame(uint8_t frameID) override;

    void log(const char *format, ...) override;
    
    unsigned int cycleCount = 0;

    RocketTelemPacket dummyPacket;
};

class ServingRadioModule
        : public RadioModule // (slaying because this class sends data through the web server so it is serving content so it is serving and if it's serving then it's also slaying)
{
public:
    WebServer *webServer;

    ServingRadioModule(int baudRate, DataLogger *logger, const QSerialPortInfo &portInfo, WebServer *server);

    ServingRadioModule(int baudRate, DataLogger *logger, WebServer *server);

    void handleReceivePacket(XBee::ReceivePacket::Struct *frame) override;

    void handleReceivePacket64Bit(XBee::ReceivePacket64Bit::Struct *frame) override;
};

class RocketTestModule : public RadioModule
{
public:
    RocketTestModule(int baudRate, DataLogger *logger, const QSerialPortInfo &portInfo) : RadioModule(baudRate, logger,
                                                                                                      portInfo)
    {}

    RocketTestModule(int baudRate, DataLogger *logger) : RadioModule(baudRate, logger)
    {}

    RocketTxPacket packet;

    void didCycle() override;
};

class PayloadTestModule : public RadioModule
{
public:
    PayloadTestModule(int baudRate, DataLogger *logger, const QSerialPortInfo &portInfo) : RadioModule(baudRate, logger,
                                                                                                       portInfo)
    {}

    PayloadTestModule(int baudRate, DataLogger *logger) : RadioModule(baudRate, logger)
    {}

    PayloadTxPacket packet;

    void didCycle() override;
};


#endif //XBEE_BACKEND_CPP_RADIOMODULE_H
