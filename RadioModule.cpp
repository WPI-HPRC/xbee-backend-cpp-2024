//
// Created by William Scheirey on 3/20/24.
//

#include "RadioModule.h"

RadioModule::RadioModule() : XBeeDevice()
{
    serialPort.start("tty.usbserial-A28DMVHS", 115200);
}

void RadioModule::serialWrite(const char *data, size_t length_bytes)
{
    serialPort.write_some(data, (int) length_bytes);
}

void RadioModule::serialRead(const char *buffer, size_t length_bytes)
{
    serialPort.read_some(buffer, length_bytes);
}

void XBeeDevice::handleReceivePacket(XBee::ReceivePacket::Struct *frame)
{

}
