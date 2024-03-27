//
// Created by William Scheirey on 3/12/24.
//

#include "XBeeDevice.h"
#include <iostream>
#include <chrono>

#define DEBUG true

int numReceived = 0;

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
    receiveFrame = new char[XBee::MaxPacketBytes];

    transmitRequestFrame = new uint8_t[XBee::MaxFrameBytes];

    atCommandFrame = new uint8_t[XBee::MaxFrameBytes];

    nodeID = new char[20];

    buffer = serialCircularBufferCreate(BUFFER_LENGTH, XBee::MaxFrameBytes);

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
    std::cout << "Send at command local" << std::endl;
    size_t index = 1;
    size_t contentLength_bytes = XBee::AtCommandTransmit::PacketBytes + commandDataSize_bytes;

    atCommandFrame[index++] = (contentLength_bytes >> 8) & 0xFF;
    atCommandFrame[index++] = contentLength_bytes & 0xFF;

    atCommandFrame[index++] = frameType; // Local AT Command Request
//    qDebug() << "Current Frame ID: " << currentFrameID;
    atCommandFrame[index++] = currentFrameID++; // Frame ID

    atCommandFrame[index++] = (command >> 8) & 0xFF;
    atCommandFrame[index++] = command & 0xFF;

    if (commandData)
    {
        memcpy(&atCommandFrame[index], commandData, commandDataSize_bytes);
    }
    sendFrame(atCommandFrame, commandDataSize_bytes + XBee::AtCommandTransmit::FrameBytes);
}

void XBeeDevice::sendAtCommandLocal(uint16_t command, const uint8_t *commandData, size_t commandDataSize_bytes)
{
    sendAtCommandLocal(XBee::FrameType::AtCommand, command, commandData, commandDataSize_bytes);
}

void XBeeDevice::sendNodeDiscoveryCommand()
{
//    setParameter(XBee::AtCommand::NodeDiscoveryBackoff, 0x20);
//    sendAtCommandLocal(AsciiToUint16('K', 'Z'), nullptr, 0);
    setParameter(XBee::AtCommand::NodeDiscoveryOptions, 0x02);
//    sendAtCommandLocal(XBee::AtCommand::NodeDiscovery, nullptr, 0);
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
    frame[size_bytes - 1] = calcChecksum(frame, size_bytes);

//    serialWrite((const char *) frame, size_bytes);


//    auto *frameStruct = new XBee::BasicFrame{};
//    frameStruct->length_bytes = size_bytes;
//    frameStruct->frame = (uint8_t *) malloc(size_bytes);
//    memcpy(frameStruct->frame, frame, size_bytes);

//    transmitFrameQueue.push( );
    serialWrite((const char *) frame, size_bytes);
}

/*
void XBeeDevice::handleReceivePacket(XBee::ReceivePacket::Struct *frame)
{
    // For now, assume that the frame is the rocket frame and don't do any other checking
    return; // Do nothing for now
}
 */

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
//    std::cout << "Addr: " << std::hex << addr << std::endl;
#if DEBUG

#endif

    receivePacketStruct->dataLength_bytes = payloadLength;
    receivePacketStruct->senderAddress = addr;
    receivePacketStruct->data = &frame[XBee::ReceivePacket::BytesBeforePayload];

    handleReceivePacket(receivePacketStruct);
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

    uint16_t parentNetworkAddress = frame[index++] << 8;
    parentNetworkAddress |= frame[index++];

    uint8_t deviceType = frame[index++];
    uint8_t status = frame[index++];

    uint16_t profileID = frame[index++] << 8;
    profileID |= frame[index++];

    uint16_t manufacturerID = frame[index++] << 8;
    manufacturerID |= frame[index++];
}

void XBeeDevice::handleAtCommandResponse(const uint8_t *frame, uint8_t length_bytes)
{
    uint8_t commandStatus = frame[XBee::AtCommandResponse::BytesBeforeCommandStatus];
//    std::cout << "Status: " << (int) commandStatus << std::endl;
    if (commandStatus != 0x00)
    {
        std::string commandString;
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
        std::cout << "Error: " << commandString.c_str() << std::endl;
        return;
    }

    uint16_t command = getAtCommand(frame);

    std::cout << (int) command;

    if (!atParamConfirmationsBeingWaitedOn.empty())
    {
        std::cout << "Good" << std::endl;
        if (atParamConfirmationsBeingWaitedOn.front() == command)
        {
            std::cout << "OK" << std::endl;
            atParamConfirmationsBeingWaitedOn.pop();
            return;
        }
    }

    switch (command)
    {
        case XBee::AtCommand::NodeDiscovery:
            handleNodeDiscoveryResponse(frame, length_bytes);
            break;

        default:
            break;
    }
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
        std::cout << "Checksum mismatch. Calculated: " << std::hex << (int) calculatedChecksum << ", Received: "
                  << std::hex << (int) receivedChecksum
                  << std::endl;
        return false;
    }
//    std::cout << "Good frame "
//              << std::dec << numReceived
//              << std::endl;
//    numReceived++;

    uint8_t frameType = frame[index++];

//    std::cout << "Frame Type: " << std::hex << static_cast<int>(frameType) << std::endl;

    switch (frameType)
    {
        case XBee::FrameType::ReceivePacket:
            parseReceivePacket(frame, lengthHigh);
            break;

        case XBee::FrameType::AtCommandResponse:
            handleAtCommandResponse(frame, lengthHigh);
            break;

        default:
            break;

    }
    return true;
}

void XBeeDevice::receive()
{
    serialRead(receiveFrame, 1);

    if (receiveFrame[0] != XBee::StartDelimiter)
    {
//        std::cout << "Wrong start delimiter: " << std::hex << (int) receiveFrame[0] << std::endl;
        return;
    }

    // Read the length of the frame (16 bits = 2 bytes) and place it directly after the start delimiter in our receive memory
    serialRead(&receiveFrame[1], 2);

    if (receiveFrame[1] != 0x00)
    {
//        std::cout << "??" << std::endl;
        return;
    }

//    std::cout << "Frame " << std::endl;

//    std::cout << "Reading length"

//    serialRead(&receiveFrame[2], 1);

    uint8_t length = receiveFrame[2];

//    std::cout << "Length: " << (int) length << std::endl;

    // Read the rest of the frame. The length represents the number of bytes between the length and the checksum.
    // The second of the two length bytes holds the real length of the frame.
    serialRead(&receiveFrame[3], length + 1);

    handleFrame((const uint8_t *) receiveFrame);
    receiveFrame[0] = 0x00;
}

void XBeeDevice::doCycle()
{
//    std::cout << "Doing cycle" << std::endl;
    // First, read frames from serial
    receive();

    // Next, handle the frames in our buffer
    /*
    for (int i = 0; i < BUFFER_LENGTH; i++)
    {
        uint8_t *frame = serialCircularBufferGetValueAtIndex(buffer, -i - 1); // Go backwards

        if (frame[0] == XBee::StartDelimiter)
        {
            handleFrame(frame);
            std::cout << "Handling frame" << std::endl;
            frame[0] = 0x00; // So we know the frame has been read
        }
        else
        {
            break; // Assume all packets have been read if we reach a frame that has already been read
        }
    }
     */

    // Finally, send out any frames that need to be sent

    while (true)
    {
        if (!atParamConfirmationsBeingWaitedOn.empty() || transmitFrameQueue.empty())
        {
            break;
        }
        XBee::BasicFrame *frame = transmitFrameQueue.front();
        uint8_t frameType = getFrameType(frame->frame);
        if (frameType == XBee::FrameType::AtCommandQueueParameterValue || frameType == XBee::FrameType::AtCommand)
        {
            atParamConfirmationsBeingWaitedOn.push(getAtCommand(frame->frame));
        }
        serialWrite((const char *) frame->frame, frame->length_bytes);
        transmitFrameQueue.pop();
        free(frame->frame);
        free(frame);
    }
}