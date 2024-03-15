//
// Created by William Scheirey on 3/12/24.
//

#ifndef XBEE_BACKEND_CPP_XBEEDEVICE_H
#define XBEE_BACKEND_CPP_XBEEDEVICE_H

#include <QObject>
#include <QSerialPort>
#include "Utility.h"
#include "XBeeUtility.h"

class XBeeDevice: public QObject
{
    Q_OBJECT
public:
    explicit XBeeDevice(QSerialPort *serialPort, QObject *parent = nullptr);
    void receive();
    void sendFrame(uint8_t *packet, size_t size_bytes);
    void sendTransmitRequestCommand(uint64_t address, const uint8_t *data, size_t size_bytes);
    void sendNodeDiscoveryCommand();

private:
    bool isProcessingPacket;

    long long lastPacketMs;

    QSerialPort *m_serialPort;

    uint8_t *transmitRequestFrame;
    uint8_t *nodeDiscoveryFrame;

    char *receivePacket;

    TelemPacket *telemPacket;

    void _receive(const uint8_t *packet);

signals:
    void dataReady(const uint8_t *data, size_t length_bytes);

};


#endif //XBEE_BACKEND_CPP_XBEEDEVICE_H
