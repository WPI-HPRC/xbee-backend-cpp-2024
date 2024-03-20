//
// Created by William Scheirey on 3/20/24.
//

#include "RadioModule.h"
#include <iostream>

RadioModule::RadioModule() : XBeeDevice()
{
    serialPort.start("/dev/tty.usbserial-A28DMVHS", 115200);
//    io_service.run();
    startLoop();
}

void RadioModule::startLoop()
{
    boost::asio::deadline_timer timer(io_service, boost::posix_time::milliseconds(10));
    timer.async_wait(boost::bind(&RadioModule::loop, this, boost::asio::placeholders::error));
//    io_service.run();
}

void RadioModule::loop(const boost::system::error_code &error)
{
    if (!error)
    {
        std::cout << "Doing cycle";
        doCycle();
        startLoop(); // Restart the timer for the next iteration
    }
    else
    {
        std::cout << "Error: " << error;
        // Handle error, if any
    }
}

void RadioModule::serialWrite(const char *data, size_t length_bytes)
{
    serialPort.write_some(data, (int) length_bytes);
}

void RadioModule::serialRead(const char *buffer, size_t length_bytes)
{
//    serialPort.read_some(buffer, length_bytes);
}

void XBeeDevice::handleReceivePacket(XBee::ReceivePacket::Struct *frame)
{

}
