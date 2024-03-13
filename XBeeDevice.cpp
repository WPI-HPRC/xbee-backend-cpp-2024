//
// Created by William Scheirey on 3/12/24.
//

#include "XBeeDevice.h"
#include "QDebug"

XBeeDevice::XBeeDevice(QSerialPort *serialPort, QObject *parent): QObject(parent), m_serialPort(serialPort)
{
    receivePacket = new char[MAX_PACKET_LENGTH];
    sendPacket = new uint8_t[MAX_PACKET_LENGTH];

    telemPacket = new TelemPacket;
}

void XBeeDevice::send(uint64_t address, const void *data, size_t size_bytes)
{
    size_t contentLength = size_bytes + 14; // +4 for start delimiter, length, and checksum, +8 for address

    auto *packet = sendPacket;

    size_t index = 0;

    packet[index++] = 0x7E; // Start delimiter

    packet[index++] = (contentLength >> 8) & 0xFF; // Length high byte
    packet[index++] = contentLength & 0xFF;        // Length low byte

    packet[index++] = 0x10; // Frame type
    packet[index++] = 0x01; // Frame ID

    for (int i = 0; i < 8; i++)
    {
        packet[index++] = (address >> ((7 - i) * 8)) & 0xFF;
    }

    packet[index++] = 0xFF; // Reserved
    packet[index++] = 0xFE; // Reserved

    packet[index++] = 0x00; // Broadcast radius

    packet[index++] = 0x00; // Options byte

    memcpy(&packet[index++], data, size_bytes);

    size_t checksum_temp = 0;

    for (size_t i = 3; i < index; i++)
    {
        checksum_temp += packet[i];
    }

    uint8_t checksum = checksum_temp & 0xFF;

    checksum = 0xFF - checksum;

    packet[contentLength - 1] = checksum;

    m_serialPort->write(QByteArray::fromRawData((const char*)packet, (long long)contentLength));
}

void XBeeDevice::handleData(const uint8_t *data, size_t length_bytes)
{
    // For now, assume telemetry packet from the rocket

    memcpy(telemPacket, data, length_bytes);

}

void XBeeDevice::_receive(const uint8_t *packet)
{
    size_t index = 0;

    if(packet[index++] != 0x7E)
    {
        qDebug() << "Wrong start delimiter: " << Qt::hex << packet[index];
        return;
    }

    uint8_t lengthLow = packet[index++];
    uint8_t lengthHigh = packet[index++];

    uint8_t frameType = packet[index++];
    uint8_t frameID = packet[index++];

    index += 12; // Skip address, reserved, broadcast radius, and options bytes

//    uint8_t dataType = packet[index++];

    index += lengthHigh;

    uint64_t checksum_temp = 0;

    for (int i = 3; i < index; i++)
    {
        checksum_temp += packet[i];
    }

    uint8_t checksum = checksum_temp & 0xFF;

    if(checksum != packet[index])
    {
        qDebug() << "Checksums do not match. Calculated: " << checksum << ", received: " << packet[index];
        return;
    }

    handleData(&packet[PACKET_HEADER_LENGTH], lengthHigh);
}

void XBeeDevice::receive()
{
    m_serialPort->read(receivePacket, MAX_PACKET_LENGTH);
    _receive((const uint8_t*)receivePacket);
}