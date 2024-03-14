//
// Created by William Scheirey on 3/12/24.
//

#include "XBeeDevice.h"
#include "QDebug"

#define DEBUG true

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

void XBeeDevice::_receive(const uint8_t *packet)
{
    size_t index = 1;

    uint8_t lengthLow = packet[index++];
    uint8_t lengthHigh = packet[index++];

    uint8_t payloadLength = lengthHigh - 12;

    uint8_t frameType = packet[index++];

    uint64_t addr = 0;

#if DEBUG
    QByteArray addrArray;
    for (int i = 0; i < 8; i++)
    {
        addrArray.append((char) packet[index + i]);
    }
    qDebug() << "Received message from: " << Qt::hex << addrArray.toHex();

#endif

    for (int i = 0; i < 8; i++)
    {
        addr = addr & (packet[index++] << 8 * i);
    }

    // Skip reserved bytes
    index += 2;

    uint8_t receiveOptions = packet[index++];

    memcpy(telemPacket, &packet[index], payloadLength);

    index += payloadLength;

    uint8_t checksum_temp = 0;

    QByteArray checksumBits;
    for (int i = 0; i < lengthHigh; i++)
    {
        checksum_temp += packet[i + 3];
        checksumBits.append((char)packet[i + 3]);
    }

    uint8_t checksum = 0xFF - checksum_temp;

    if(checksum != packet[index])
    {
#if DEBUG
        qDebug() << "Checksums do not match. Calculated: " << Qt::hex << checksum << "Received: " << packet[index];
        qDebug() << "Packet received: " << QByteArray::fromRawData(receivePacket, lengthHigh + 4).toHex();
        qDebug() << "Checksum bits: " << checksumBits.toHex();
#endif
        return;
    }

    emit dataReady(&packet[15], payloadLength);
}

void XBeeDevice::receive()
{
    if(isProcessingPacket)
        return;
    isProcessingPacket = true;

    m_serialPort->read(receivePacket, 1);

    // Check for start delimiter
    if(receivePacket[0] != 0x7E)
    {
        isProcessingPacket = false;
        return;
    }

    // Read the length of the packet (16 bits = 2 bytes) and place it directly after the start delimiter in our receive memory
    m_serialPort->read(&receivePacket[1], 2);

    // Read the rest of the packet. The length represents the number of bytes between the length and the checksum.
    // The second of the two length bytes holds the real length of the packet.
    m_serialPort->read(&receivePacket[3], receivePacket[2] + 1);

    isProcessingPacket = false;

    _receive((const uint8_t*)receivePacket);
}