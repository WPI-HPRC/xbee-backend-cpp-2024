//
// Created by William Scheirey on 3/12/24.
//

#include "XBeeDevice.h"
#include "QDebug"
#include <QDateTime>

#define DEBUG true

uint8_t calcChecksum(const uint8_t *packet, uint8_t size_bytes)
{
    uint8_t sum = 0;

    for (uint8_t i = 0; i < size_bytes; i++)
    {
        sum += packet[3 + i]; // Skip start delimiter and length bytes
    }

    return 0xFF - sum;
}

XBee::BasicFrame XBeeDevice::createSetATParamValueFrame(uint16_t command, uint8_t value)
{

}

XBeeDevice::XBeeDevice(QSerialPort *serialPort, QObject *parent): QObject(parent), m_serialPort(serialPort)
{
    receiveFrame = new char[XBee::MaxPacketBytes];

    transmitRequestFrame = new uint8_t[XBee::MaxFrameBytes];

    atCommandFrame = new uint8_t[XBee::MaxFrameBytes];

    telemPacket = new TelemPacket;

    test = new uint8_t[XBee::MaxFrameBytes];

    nodeID = new char[20];
}

void XBeeDevice::queryParameter(uint16_t parameter)
{
    sendAtCommandLocal(parameter, nullptr, 0);
}

void XBeeDevice::setParameter(uint16_t parameter, const uint8_t value)
{
    sendAtCommandLocal(parameter, &value, 1);
}

void XBeeDevice::setParameter(uint16_t parameter, const uint8_t *value, size_t valueSize_bytes)
{
    sendAtCommandLocal(parameter, value, valueSize_bytes);
}

void XBeeDevice::sendAtCommandLocal(uint16_t command, const uint8_t *commandData, size_t commandDataSize_bytes)
{
    size_t index = 1;
    size_t contentLength_bytes = XBee::AtCommandTransmit::PacketBytes + commandDataSize_bytes;

    atCommandFrame[index++] = (contentLength_bytes >> 8) & 0xFF;
    atCommandFrame[index++] = contentLength_bytes & 0xFF;

    atCommandFrame[index++] = 0x08; // Local AT Command Request
    atCommandFrame[index++] = 0x01; // Frame ID

    atCommandFrame[index++] = (command >> 8) & 0xFF;
    atCommandFrame[index++] = command & 0xFF;

    if(commandData)
    {
        memcpy(&atCommandFrame[index], commandData, commandDataSize_bytes);
    }
    sendFrame(atCommandFrame, commandDataSize_bytes + XBee::AtCommandTransmit::FrameBytes);
}

void XBeeDevice::sendNodeDiscoveryCommand()
{
    // Query it first
//    setParameter(XBee::AtCommand::NodeDiscoveryBackoff, 0x20);
//    sendAtCommandLocal(AsciiToUint16('K', 'Z'), nullptr, 0);
//    setParameter(XBee::AtCommand::NodeDiscoveryOptions, 0x02);
    sendAtCommandLocal(XBee::AtCommand::NodeDiscovery, nullptr, 0);
}

void XBeeDevice::sendTransmitRequestCommand(uint64_t address, const uint8_t *data, size_t size_bytes)
{
    size_t contentLength_bytes = size_bytes + XBee::TransmitRequest::PacketBytes;
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
    transmitRequestFrame[index++] = 0x00; // Transmit options. Use "TO" value (in parameters of the radio itself)

    memcpy(&transmitRequestFrame[index], data, size_bytes);

    sendFrame(transmitRequestFrame, size_bytes + XBee::TransmitRequest::FrameBytes);
}

void XBeeDevice::sendFrame(uint8_t *packet, size_t size_bytes)
{
    packet[0] = 0x7E; // Start delimiter;
    packet[size_bytes - 1] = calcChecksum(packet, size_bytes);

#if DEBUG
    qDebug() << "Sending packet " << QByteArray::fromRawData((const char *)packet, (long long)size_bytes).toHex();
#endif

    m_serialPort->write((const char*)packet, (long long)size_bytes);
}

void XBeeDevice::handleReceivePacket(XBee::ReceivePacket::Struct *packet)
{
    // For now, assume that the packet is the rocket packet and don't do any other checking
    emit dataReady(packet->data, packet->dataLength_bytes);
}

void XBeeDevice::parseReceivePacket(const uint8_t *packet, uint8_t length_bytes)
{
    uint8_t payloadLength = length_bytes - XBee::ReceivePacket::PacketBytes; // Subtract the number of base packet bytes from the total number of packet bytes

    uint64_t addr = 0;

    uint8_t index = XBee::ReceivePacket::BytesBeforeAddress;

    for (int i = 0; i < 8; i++)
    {
        addr = addr | (packet[index++] << 8 * i);
    }
#if DEBUG
    QByteArray addressBytes;
    for (int i = 0; i < 8; i++)
    {
        addressBytes.append((char)packet[i + XBee::ReceivePacket::BytesBeforeAddress]);
    }
//    qDebug() << "Received message from: " <<  addressBytes.toHex();

#endif

    receivePacketStruct->dataLength_bytes = payloadLength;
    receivePacketStruct->senderAddress = addr;
    receivePacketStruct->data = &packet[XBee::ReceivePacket::BytesBeforePayload];

    handleReceivePacket(receivePacketStruct);
}

uint16_t XBeeDevice::getAtCommand(const uint8_t *packet)
{
    return packet[XBee::AtCommandResponse::BytesBeforeCommand] << 8 | packet[XBee::AtCommandResponse::BytesBeforeCommand + 1];
}

void XBeeDevice::handleNodeDiscoveryResponse(const uint8_t *packet, uint8_t length_bytes)
{
    for (int i = 0; i < length_bytes + XBee::FrameBytes; i++)
    {
//        qDebug() << Qt::hex << packet[i];
    }
    size_t index = XBee::AtCommandResponse::BytesBeforeCommandData + 2; // Add two to skip the MY parameter

    uint32_t serialHigh = 0;
    for (int i = 0; i < 4; i++)
    {
        serialHigh = serialHigh | (packet[index++] << 8 * (7-i));
    }

    uint32_t serialLow = 0;
    for (int i = 0; i < 4; i++)
    {
        serialLow = serialLow | (packet[index++] << 8 * (7-i));
    }

    uint64_t serialNumber = serialHigh;
    serialNumber = serialNumber << 32 | serialLow;

//    qDebug() << "Serial number: " << Qt::hex << serialNumber;

//    uint8_t signalStrength = packet[index++];

    int iDLength = 0;

    for (int i = 0; i < 20; i++)
    {
        uint8_t byte = packet[index++];

        if(byte == 0x00)
        {
            if(i > 0)
                iDLength = i; // Remember that there is an extra byte in the packet; this null character
            break;
        }

        nodeID[i] = (char)byte;
    }

//    qDebug() << "Node ID: " << QString::fromUtf8(nodeID, iDLength);

    uint16_t parentNetworkAddress = packet[index++] << 8;
    parentNetworkAddress |= packet[index++];

//    qDebug() << "Parent address: " << Qt::hex << parentNetworkAddress;

    uint8_t deviceType = packet[index++];
//    qDebug() << "Device Type: " << Qt::hex << deviceType;

    uint8_t status = packet[index++];
//    qDebug() << "Status: " << Qt::hex << status;

    uint16_t profileID = packet[index++] << 8;
    profileID |= packet[index++];
//    qDebug() << "Profile ID: " << Qt::hex << profileID;

    uint16_t manufacturerID = packet[index++] << 8;
    manufacturerID |= packet[index++];

//    qDebug() << "Manufacturer ID: " << Qt::hex << manufacturerID;

/*
    if(length_bytes - iDLength > XBee::AtCommandResponse::NodeDiscovery::FrameBytes)
    {
        qDebug() << "Length - ID: " << length_bytes - iDLength;
        qDebug() << "Frame length: " << XBee::AtCommandResponse::NodeDiscovery::FrameBytes;
        qDebug() << "Extra bytes!";
    }
    */

    qDebug() << "Found device " << QString::fromUtf8(nodeID, iDLength) << "-" << Qt::hex << serialNumber;
}

void XBeeDevice::handleNodeDiscoveryOptionsResponse(const uint8_t *packet, uint8_t length_bytes)
{
    uint8_t dataLength = length_bytes - XBee::AtCommandResponse::PacketBytes;

    if(dataLength > 1)
    {
        qDebug() << "More than 1 byte...?";
        return;
    }

    uint8_t value = packet[XBee::AtCommandResponse::BytesBeforeCommandData];

    if(value != 0x02)
    {
        setParameter(XBee::AtCommand::NodeDiscoveryOptions, 0x02);
    }
}

void XBeeDevice::handleAtCommandResponse(const uint8_t *packet, uint8_t length_bytes)
{
    uint8_t commandStatus = packet[XBee::AtCommandResponse::BytesBeforeCommandStatus];
    if(commandStatus != 0x00)
    {
        QString commandString = "";
        switch (commandStatus)
        {
            case XBee::AtCommand::Error:
                commandString = "Error in command";
                break;
            case XBee::AtCommand::InvalidCommand:
                commandString = "Invalid command";
                break;
            case XBee::AtCommand::InvalidParameter:
                commandString = "Invalid parameter";
                break;
            default:
                commandString = "You have broken physics";
                break;
        }
        qDebug() << "Command status: " << commandString << " code: " << Qt::hex << commandStatus;
        return;
    }

    uint16_t command = getAtCommand(packet);

    if(length_bytes == XBee::AtCommandResponse::PacketBytes)
    {
        qDebug() << "Response for " << Qt::hex << command << ": OK";
        return;
    }

    switch (command)
    {
        case XBee::AtCommand::NodeDiscovery:
            handleNodeDiscoveryResponse(packet, length_bytes);
            break;
        case XBee::AtCommand::NodeDiscoveryOptions:
            handleNodeDiscoveryOptionsResponse(packet, length_bytes);
            break;

        default:
            qDebug() << "Unimplemented At Command response: " << Qt::hex << command;
            break;
    }
}

bool XBeeDevice::_receive(const uint8_t *packet)
{
    size_t index = 1; // Skip start delimiter

    uint8_t lengthLow = packet[index++];
    uint8_t lengthHigh = packet[index++];
    uint8_t frameType = packet[index++];

    uint8_t calculatedChecksum = calcChecksum(packet, lengthHigh);
    uint8_t receivedChecksum = packet[lengthHigh + XBee::FrameBytes - 1];

    if(calculatedChecksum != receivedChecksum)
    {
#if DEBUG
        /*
        qDebug() << "Length low:  " << lengthLow;
        qDebug() << "Length high: " << lengthHigh;
        qDebug() << "Frame Type:  " << Qt::hex << frameType;

        QByteArray checksumBytes;
        for (int i = 0; i < lengthHigh - 1; i++)
        {
            checksumBytes.append((char)packet[i + 3]);
        }

        for (int i = 0; i < lengthHigh + XBee::FrameBytes; i++)
        {
            qDebug() << "Index " << i << ": " << Qt::hex << packet[i];
        }
        qDebug() << "Checksum index: " << lengthHigh + XBee::FrameBytes - 1;
        qDebug() << "Checksum: " << Qt::hex << packet[lengthHigh + XBee::FrameBytes - 1];
        qDebug() << "Checksum bytes:        " << checksumBytes.toHex();
         */

//        qDebug() << "Checksums do not match. Calculated: " << Qt::hex << calculatedChecksum << "Received: " << packet[index];
//        qDebug() << "Packet received: " << QByteArray::fromRawData(receiveFrame, lengthHigh + XBee::FrameBytes).toHex();

#endif
//        return false;
    }

    memcpy(test, packet, lengthHigh + XBee::FrameBytes);

//    qDebug() << "Packet received: " << QByteArray::fromRawData(receiveFrame, lengthHigh + XBee::FrameBytes).toHex();
//    qDebug() << "Checksum bits: " << checksumBits.toHex();

    switch (frameType)
    {
        case XBee::FrameType::ReceivePacket:
            parseReceivePacket(test, lengthHigh);
            break;

        case XBee::FrameType::AtCommandResponse:
            handleAtCommandResponse(test, lengthHigh);
            break;

        default:
            qDebug() << "Received unimplemented frame type: " << Qt::hex << frameType;

    }
    return true;
}

void XBeeDevice::receive()
{
    while (true)
    {
        if (isProcessingPacket)
        {
            qDebug() << "Already processing packet";
            return;
        }

        isProcessingPacket = true;
        m_serialPort->read(receiveFrame, 1);

        // Check for start delimiter
        if (receiveFrame[0] != XBee::StartDelimiter)
        {
#if DEBUG
            if(receiveFrame[0] != 0x00)
            {
//                qDebug() << "Wrong start delimiter: " << Qt::hex << (uint8_t) receiveFrame[0];
            }
#endif

            isProcessingPacket = false;
            return;
        }

        // Read the length of the packet (16 bits = 2 bytes) and place it directly after the start delimiter in our receive memory
        m_serialPort->read(&receiveFrame[1], 2);

        // Read the rest of the packet. The length represents the number of bytes between the length and the checksum.
        // The second of the two length bytes holds the real length of the packet.
        m_serialPort->read(&receiveFrame[3], receiveFrame[2] + 1);

        bool success = _receive((const uint8_t *) receiveFrame);
        isProcessingPacket = false;
        if(!success)
            return;
        return;
//        qDebug("Looping");
    }

}