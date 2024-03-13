//
// Created by William Scheirey on 3/12/24.
//

#ifndef XBEE_BACKEND_CPP_XBEEDEVICE_H
#define XBEE_BACKEND_CPP_XBEEDEVICE_H

#include <QObject>
#include <QSerialPort>
#include "Utility.h"

#define PACKET_HEADER_LENGTH 17
#define MAX_PAYLOAD_LENGTH 256
#define MAX_PACKET_LENGTH (MAX_PAYLOAD_LENGTH + PACKET_HEADER_LENGTH)

class XBeeDevice: public QObject
{
    Q_OBJECT
public:
    explicit XBeeDevice(QSerialPort *serialPort, QObject *parent = nullptr);
    void receive();
    void send(uint64_t address, const void *data, size_t size_bytes);

private:
    QSerialPort *m_serialPort;

    uint8_t *sendPacket;
    char *receivePacket;

    TelemPacket *telemPacket;

    void _receive(const uint8_t *packet);
    void handleData(const uint8_t *data, size_t length_bytes);

signals:
    void dataReady(const uint8_t *data, size_t length_bytes);

};


#endif //XBEE_BACKEND_CPP_XBEEDEVICE_H
