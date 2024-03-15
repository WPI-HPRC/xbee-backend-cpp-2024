//
// Created by William Scheirey on 3/12/24.
//

#include "XBeeDevice.h"
#include "QDebug"
#include <QDateTime>

#define DEBUG true

uint8_t calcChecksum(const uint8_t *packet, size_t size_bytes)
{
    uint8_t sum = 0;

    for (size_t i = 0; i < size_bytes; i++)
    {
        sum += packet[3 + i]; // Skip start delimiter and length bytes
    }

    return 0xFF - sum;
}

XBeeDevice::XBeeDevice(QSerialPort *serialPort, QObject *parent): QObject(parent), m_serialPort(serialPort)
{
    receivePacket = new char[MAX_FRAME_LENGTH];

    transmitRequestFrame = new uint8_t[MAX_FRAME_LENGTH];

    nodeDiscoveryFrame = new uint8_t[MAX_FRAME_LENGTH];

    telemPacket = new TelemPacket;
}

void XBeeDevice::sendNodeDiscoveryCommand()
{
    size_t index = 1;
    size_t contentLength_bytes = AT_COMMAND_BYTES + NODE_DISCOVERY_EXTRA_BYTES;

    nodeDiscoveryFrame[index++] = (contentLength_bytes >> 8) & 0xFF;
    nodeDiscoveryFrame[index++] = contentLength_bytes & 0xFF;

    nodeDiscoveryFrame[index++] = 0x08; // Local AT Command Request
    nodeDiscoveryFrame[index++] = 0x01; // Frame ID

    // These two bits represent the actual AT command.
    // "ND" = "Node Discovery"
    nodeDiscoveryFrame[index++] = 'N';
    nodeDiscoveryFrame[index++] = 'D';

    sendFrame(nodeDiscoveryFrame, contentLength_bytes);
}

void XBeeDevice::sendTransmitRequestCommand(uint64_t address, const uint8_t *data, size_t size_bytes)
{
    size_t contentLength_bytes = size_bytes + TRANSMIT_REQUEST_EXTRA_BYTES;
    size_t index = 1; // skip first byte (start delimiter)

    transmitRequestFrame[index++] = (contentLength_bytes >> 8) & 0xFF;
    transmitRequestFrame[index++] = contentLength_bytes & 0xFF;

    transmitRequestFrame[index++] = 0x10; // Transmit Request
    transmitRequestFrame[index++] = 0x01; // Frame ID

    for (int i = 0; i < 8; i++)
    {
        transmitRequestFrame[index++] = (address >> ((7 - i) * 8)) & 0xFF;
    }

    transmitRequestFrame[index++] = 0xFF; // Reserved
    transmitRequestFrame[index++] = 0xFE; // Reserved

    transmitRequestFrame[index++] = 0x00; // Broadcast radius
    transmitRequestFrame[index++] = 0x00; // Transmit options. Use TO value (in parameters of the radio itself)

    memcpy(&transmitRequestFrame[index], data, size_bytes);

    sendFrame(transmitRequestFrame, contentLength_bytes);
}

void XBeeDevice::sendFrame(uint8_t *packet, size_t size_bytes)
{
    packet[0] = 0x7E; // Start delimiter;
    packet[3 + size_bytes] = calcChecksum(packet, size_bytes);

#if DEBUG
    qDebug() << QByteArray::fromRawData((const char *)packet, (long long)size_bytes + 4).toHex();
#endif

    m_serialPort->write((const char*)packet, (long long)size_bytes + 4);
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

    for (int i = 0; i < lengthHigh; i++)
    {
        checksum_temp += packet[i + 3];
    }

    uint8_t checksum = 0xFF - checksum_temp;

    if(checksum != packet[index])
    {
#if DEBUG
        QByteArray checksumBits;
        for (int i = 0; i < lengthHigh; i++)
        {
            checksumBits.append((char)packet[i + 3]);
        }
        qDebug() << "Checksums do not match. Calculated: " << Qt::hex << checksum << "Received: " << packet[index];
        qDebug() << "Packet received: " << QByteArray::fromRawData(receivePacket, lengthHigh + 4).toHex();
        qDebug() << "Checksum bits: " << checksumBits.toHex();
#endif
//        return;
    }

//    qDebug() << "Received packet: " << ((TelemPacket *)(&packet[15]))->timestamp;

    emit dataReady(&packet[15], payloadLength);
}

void XBeeDevice::receive()
{
    if(isProcessingPacket)
    {
        qDebug("Already receiving");
        return;
    }

    isProcessingPacket = true;

    m_serialPort->read(receivePacket, 1);

    // Check for start delimiter
    if(receivePacket[0] != 0x7E)
    {
        isProcessingPacket = false;
        return;
    }

    long long currentMs = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    qDebug() << "Time between packets: " << currentMs - lastPacketMs;

    lastPacketMs = currentMs;

    // Read the length of the packet (16 bits = 2 bytes) and place it directly after the start delimiter in our receive memory
    m_serialPort->read(&receivePacket[1], 2);

    // Read the rest of the packet. The length represents the number of bytes between the length and the checksum.
    // The second of the two length bytes holds the real length of the packet.
    m_serialPort->read(&receivePacket[3], receivePacket[2] + 1);

    isProcessingPacket = false;

    _receive((const uint8_t*)receivePacket);
}