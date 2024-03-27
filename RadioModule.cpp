//
// Created by William Scheirey on 3/20/24.
//

#include "RadioModule.h"
#include <iostream>

RadioModule::RadioModule() : XBeeDevice()
{
    serialPort = new SerialPort();
    serialPort->start("/dev/tty.usbserial-A28DMVHS", 115200);
    std::cout << "Starting " << std::endl;

    webServer = new WebServer(8001);
}

void RadioModule::serialWrite(const char *data, size_t length_bytes)
{
    std::cout << "Writing" << std::endl;
    serialPort->write_some(data, (int) length_bytes);
}

void RadioModule::packetRead()
{
    serialPort->packetsNotYetRead -= 1;
}

void RadioModule::serialRead(char *buffer, size_t length_bytes)
{
    serialPort->read(buffer, length_bytes);
}

void RadioModule::handleReceivePacket(XBee::ReceivePacket::Struct *frame)
{
//    std::cout << "Length: " << std::dec << frame->dataLength_bytes << std::endl;

    auto *packet = (TelemPacket *) frame->data;

    webServer->dataReady(frame->data, frame->dataLength_bytes);

//    std::cout << "timestamp: " << std::dec << packet->timestamp << std::endl;
}
