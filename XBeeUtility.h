//
// Created by William Scheirey on 3/15/24.
//

#include <cstdint>

#ifndef XBEE_BACKEND_CPP_XBEEUTILITY_H
#define XBEE_BACKEND_CPP_XBEEUTILITY_H

#endif //XBEE_BACKEND_CPP_XBEEUTILITY_H

namespace XBee
{

    const uint8_t MaxPacketBytes = 255;
    const uint8_t FrameBytes = 4; // +1 for start delimiter, +2 for length, +1 for checksum
    const uint16_t MaxFrameBytes = MaxPacketBytes + FrameBytes;

    namespace AtCommand
    {
        const uint8_t PacketBytes = 4; // +1 for frame type, +1 for frame ID, +2 for AT command

        namespace NodeDiscovery
        {
            const uint8_t PacketBytes = AtCommand::PacketBytes + 0;
            const uint8_t FrameBytes = XBee::FrameBytes + PacketBytes;
        }
    }

    namespace TransmitRequest
    {
        const uint8_t PacketBytes = 14; // +1 for frame type, +1 for frame ID, +8 for destination address, +2 for reserved, +1 for broadcast radius, +1 for transmit options
        const uint8_t FrameBytes = XBee::FrameBytes + PacketBytes;
    }
}