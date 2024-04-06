//
// Created by William Scheirey on 3/12/24.
//

#include "XBeeDevice.h"
#include <iostream>

#define DEBUG true

int goodFrameNumber = 0;

uint8_t calcChecksum(const uint8_t *packet, uint8_t size_bytes)
{
    uint8_t sum = 0;

    for (uint8_t i = 0; i < size_bytes; i++)
    {
        sum += packet[3 + i]; // Skip start delimiter and length bytes
    }

    return 0xFF - sum;
}

uint8_t getFrameType(const uint8_t *packet)
{
    return packet[3];
}

XBeeDevice::XBeeDevice()
{
    receiveFrame = new uint8_t[XBee::MaxPacketBytes];

    transmitRequestFrame = new uint8_t[XBee::MaxFrameBytes];

    atCommandFrame = new uint8_t[XBee::MaxFrameBytes];

    nodeID = new char[20];

    buffer = circularBufferCreate(BUFFER_LENGTH, XBee::MaxFrameBytes);

    atParamConfirmationsBeingWaitedOn = circularQueueCreate<uint16_t>(256);

    transmitFrameQueue = circularQueueCreate<XBee::BasicFrame>(16);


    currentFrameID = 1;
}

void XBeeDevice::queryParameter(uint16_t parameter)
{
    sendAtCommandLocal(parameter, nullptr, 0);
}

void XBeeDevice::setParameter(uint16_t parameter, const uint8_t value)
{
    setParameter(parameter, &value, 1);
}

void XBeeDevice::setParameter(uint16_t parameter, const uint8_t *value, size_t valueSize_bytes)
{
    // Could queue it, but just set it directly right now
    queueAtCommandLocal(parameter, value, valueSize_bytes);
//    write();
//    applyChanges();
}

void XBeeDevice::applyChanges()
{
    sendAtCommandLocal(XBee::AtCommand::ApplyChanges, nullptr, 0);
}

void XBeeDevice::write()
{
    sendAtCommandLocal(XBee::AtCommand::Write, nullptr, 0);
}

void XBeeDevice::queueAtCommandLocal(uint16_t command, const uint8_t *commandData, size_t commandDataSize_bytes)
{
    sendAtCommandLocal(XBee::FrameType::AtCommandQueueParameterValue, command, commandData, commandDataSize_bytes);
}

void XBeeDevice::sendAtCommandLocal(uint8_t frameType, uint16_t command, const uint8_t *commandData,
                                    size_t commandDataSize_bytes)
{
    size_t index = 1;
    size_t contentLength_bytes = XBee::AtCommandTransmit::PacketBytes + commandDataSize_bytes;

    atCommandFrame[index++] = (contentLength_bytes >> 8) & 0xFF;
    atCommandFrame[index++] = contentLength_bytes & 0xFF;

    atCommandFrame[index++] = frameType; // Local AT Command Request
    atCommandFrame[index++] = currentFrameID++; // Frame ID

    atCommandFrame[index++] = (command >> 8) & 0xFF;
    atCommandFrame[index++] = command & 0xFF;

    if (commandData)
    {
        memcpy(&atCommandFrame[index], commandData, commandDataSize_bytes);
    }

//    circularQueuePush(atParamConfirmationsBeingWaitedOn, command);
    sendFrame(atCommandFrame, commandDataSize_bytes + XBee::AtCommandTransmit::FrameBytes);
}

void XBeeDevice::sendAtCommandLocal(uint16_t command, const uint8_t *commandData, size_t commandDataSize_bytes)
{
    sendAtCommandLocal(XBee::FrameType::AtCommand, command, commandData, commandDataSize_bytes);
}

void XBeeDevice::sendNodeDiscoveryCommand()
{
    setParameter(XBee::AtCommand::NodeDiscoveryBackoff, 0x20);
    setParameter(XBee::AtCommand::NodeDiscoveryOptions, 0x02);
    sendAtCommandLocal(XBee::AtCommand::NodeDiscovery, nullptr, 0);
}

void XBeeDevice::sendTransmitRequestCommand(uint64_t address, const uint8_t *data, size_t size_bytes)
{
    size_t contentLength_bytes = size_bytes + XBee::TransmitRequest::PacketBytes;
    size_t index = 1; // skip first byte (start delimiter)

    transmitRequestFrame[index++] = (contentLength_bytes >> 8) & 0xFF;
    transmitRequestFrame[index++] = contentLength_bytes & 0xFF;

    transmitRequestFrame[index++] = 0x10; // Transmit Request
    transmitRequestFrame[index++] = currentFrameID++; // Frame ID

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

void XBeeDevice::sendFrame(uint8_t *frame, size_t size_bytes)
{
    frame[0] = 0x7E; // Start delimiter;

    uint8_t checksum = calcChecksum(frame, size_bytes - XBee::FrameBytes);
    frame[size_bytes - 1] = checksum;

    if (getFrameType(frame) == XBee::FrameType::TransmitRequest && sendTransmitRequestsImmediately)
    {
        writeBytes((const char *) frame, size_bytes);
    }
    else
    {
        tempFrame.length_bytes = size_bytes;
        memcpy(tempFrame.frame, frame, size_bytes);

        circularQueuePush(transmitFrameQueue, tempFrame);
    }
}

void XBeeDevice::parseReceivePacket(const uint8_t *frame, uint8_t length_bytes)
{
    uint8_t payloadLength = length_bytes -
                            XBee::ReceivePacket::PacketBytes; // Subtract the number of base frame bytes from the total number of frame bytes

    uint64_t addr = 0;

    uint8_t index = XBee::ReceivePacket::BytesBeforeAddress;

    for (int i = 0; i < 8; i++)
    {
        addr = addr | (frame[index++] << 8 * i);
    }

    receivePacketStruct->dataLength_bytes = payloadLength;
    receivePacketStruct->senderAddress = addr;
    receivePacketStruct->data = &frame[XBee::ReceivePacket::BytesBeforePayload];

    handleReceivePacket(receivePacketStruct);
}

void XBeeDevice::parseReceivePacket64Bit(const uint8_t *frame, uint8_t length_bytes)
{
    uint8_t payloadLength = length_bytes -
                            XBee::ReceivePacket64Bit::PacketBytes; // Subtract the number of base frame bytes from the total number of frame bytes

    uint64_t addr = 0;

    uint8_t index = XBee::ReceivePacket64Bit::BytesBeforeAddress;

    for (int i = 0; i < 8; i++)
    {
        addr = addr | (frame[index++] << 8 * i);
    }

    receivePacket64BitStruct->dataLength_bytes = payloadLength;
    receivePacket64BitStruct->senderAddress = addr;
    receivePacket64BitStruct->data = &frame[XBee::ReceivePacket64Bit::BytesBeforePayload];
    receivePacket64BitStruct->negativeRssi = frame[XBee::ReceivePacket64Bit::BytesBeforeRssi];

    handleReceivePacket64Bit(receivePacket64BitStruct);
}

uint16_t XBeeDevice::getAtCommand(const uint8_t *frame)
{
    return frame[XBee::AtCommandResponse::BytesBeforeCommand] << 8 |
           frame[XBee::AtCommandResponse::BytesBeforeCommand + 1];
}

void XBeeDevice::handleNodeDiscoveryResponse(const uint8_t *frame, uint8_t length_bytes)
{
    size_t index = XBee::AtCommandResponse::BytesBeforeCommandData + 2; // Add two to skip the MY parameter

    uint32_t serialHigh = 0;
    for (int i = 0; i < 4; i++)
    {
        serialHigh = serialHigh | (frame[index++] << 8 * (7 - i));
    }

    uint32_t serialLow = 0;
    for (int i = 0; i < 4; i++)
    {
        serialLow = serialLow | (frame[index++] << 8 * (7 - i));
    }

    uint64_t serialNumber = serialHigh;
    serialNumber = serialNumber << 32 | serialLow;

    int iDLength = 0;

    for (int i = 0; i < 20; i++)
    {
        uint8_t byte = frame[index++];

        if (byte == 0x00)
        {
            if (i > 0)
            {
                iDLength = i;
            } // Remember that there is an extra byte in the frame; this null character
            break;
        }

        nodeID[i] = (char) byte;
    }

    log("Found device: ");
    for (int i = 0; i < iDLength; i++)
    {
        log("%c", nodeID[i]);
    }
    log(" - %016llx\n", static_cast<unsigned long long>(serialNumber));


    uint16_t parentNetworkAddress = frame[index++] << 8;
    parentNetworkAddress |= frame[index++];

    uint8_t deviceType = frame[index++];
    uint8_t status = frame[index++];

    uint16_t profileID = frame[index++] << 8;
    profileID |= frame[index++];

    uint16_t manufacturerID = frame[index++] << 8;
    manufacturerID |= frame[index++];
}

void XBeeDevice::_handleAtCommandResponse(const uint8_t *frame, uint8_t length_bytes, bool paramWasBeingWaitedOn)
{
    uint16_t command = getAtCommand(frame);
    switch (command)
    {
        case XBee::AtCommand::NodeDiscovery:
            handleNodeDiscoveryResponse(frame, length_bytes);
            return;

        default:
            break;
    }

    log("AT command response for %c%c: ", (command & 0xFF00) >> 8, command & 0x00FF);
    for (uint8_t i = 0; i < length_bytes - XBee::AtCommandResponse::PacketBytes; i++)
    {
        log("%02x ", (int) (frame[XBee::AtCommandResponse::BytesBeforeCommandData + i] & 0xFF));
    }

    log("\n");
}

void XBeeDevice::handleAtCommandResponse(const uint8_t *frame, uint8_t length_bytes)
{
    uint8_t commandStatus = frame[XBee::AtCommandResponse::BytesBeforeCommandStatus];

    uint16_t command = getAtCommand(frame);

    bool paramWasBeingWaitedOn = false;
    if (!isCircularQueueEmpty(atParamConfirmationsBeingWaitedOn))
    {
        uint16_t commandBeingWaitedOn = 0;
        circularQueuePeek(atParamConfirmationsBeingWaitedOn, &commandBeingWaitedOn, 1);
        if (commandBeingWaitedOn == command)
        {
            paramWasBeingWaitedOn = true;
            circularQueuePop(atParamConfirmationsBeingWaitedOn, &commandBeingWaitedOn, 1);

            if (commandStatus != 0x00)
            {
                log("AT command response for %c%c: ", (command & 0xFF00) >> 8, command & 0x00FF);
                switch (commandStatus)
                {
                    case XBee::AtCommand::Error:
                        log("Error in command\n");
                        break;
                    case XBee::AtCommand::InvalidCommand:
                        log("Invalid command\n");
                        break;
                    case XBee::AtCommand::InvalidParameter:
                        log("Invalid parameter\n");
                        break;
                    default:
                        log("You have broken physics\n");
                        break;
                }
                return;
            }
            else if (length_bytes == XBee::AtCommandResponse::PacketBytes)
            {
                log("AT command response for %c%c: OK\n", (command & 0xFF00) >> 8, command & 0x00FF);
                return;
            }
        }
    }

    _handleAtCommandResponse(frame, length_bytes, paramWasBeingWaitedOn);
}

bool XBeeDevice::handleFrame(const uint8_t *frame)
{
    size_t index = 1; // Skip start delimiter

    uint8_t lengthLow = frame[index++];
    uint8_t lengthHigh = frame[index++];

    uint8_t calculatedChecksum = calcChecksum(frame, lengthHigh);
    uint8_t receivedChecksum = frame[lengthHigh + XBee::FrameBytes - 1];

    packetRead();

    if (calculatedChecksum != receivedChecksum)
    {
        log("Checksum mismatch. Calculated: %02x, received: %02x\n", (int) (calculatedChecksum & 0xFF),
            (int) (receivedChecksum & 0xFF));
        incorrectChecksum(calculatedChecksum, receivedChecksum);
        return false;
    }

    uint8_t frameType = frame[index++];

    switch (frameType)
    {
        case XBee::FrameType::ReceivePacket:
            parseReceivePacket(frame, lengthHigh);
            break;

        case XBee::FrameType::ReceivePacket64Bit:
            parseReceivePacket64Bit(frame, lengthHigh);
            break;

        case XBee::FrameType::AtCommandResponse:
            handleAtCommandResponse(frame, lengthHigh);
            break;

        default:
            log("Unrecognized frame type: %02x\n", (int) (frameType & 0xFF));

    }
    return true;
}

bool XBeeDevice::receive()
{
    readBytes(receiveFrame, 1);

    if (receiveFrame[0] != XBee::StartDelimiter)
    {
        return false;
    }

    // Read the length of the frame (16 bits = 2 bytes) and place it directly after the start delimiter in our receive memory
    readBytes(&receiveFrame[1], 2);

    if (receiveFrame[1] != 0x00)
    {
        return false;
    }

    uint8_t length = receiveFrame[2];

    // Read the rest of the frame. The length represents the number of bytes between the length and the checksum.
    // The second of the two length bytes holds the real length of the frame.
    readBytes(&receiveFrame[3], length + 1);

    handleFrame(receiveFrame);

    receiveFrame[0] = 0;

    return true;
}

void XBeeDevice::doCycle()
{
    // First, read frames from serial
    bool receivedPacket = false;
    do
    {
        receivedPacket = receive();
    } while (receivedPacket);

    // Next, send out any frames that need to be sent

    while (true)
    {
        if (!isCircularQueueEmpty(atParamConfirmationsBeingWaitedOn) || isCircularQueueEmpty(transmitFrameQueue))
        {
            break;
        }

        circularQueuePop(transmitFrameQueue, &tempFrame, 1);
        uint8_t frameType = getFrameType(tempFrame.frame);
        if (frameType == XBee::FrameType::AtCommandQueueParameterValue || frameType == XBee::FrameType::AtCommand)
        {
            circularQueuePush(atParamConfirmationsBeingWaitedOn, getAtCommand(tempFrame.frame));
        }
        writeBytes((const char *) tempFrame.frame, tempFrame.length_bytes);
    }
}