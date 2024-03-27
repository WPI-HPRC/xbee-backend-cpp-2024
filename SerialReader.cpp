//
// Created by William Scheirey on 2/28/24.
//

#define connectHelper(method_signature) connect(m_serialPort, SIGNAL(method_signature), this, SLOT(method_signature))

#include <iostream>
#include "SerialReader.h"

#define DEBUG true

RadioModule *SerialReader::radioModule = nullptr;

boost::asio::io_service io_service;
boost::posix_time::milliseconds interval(30);  // 1 second
boost::asio::deadline_timer timer(io_service, interval);

void tick(const boost::system::error_code & /*e*/)
{
    // Reschedule the timer for 1 second in the future:
    timer.expires_at(timer.expires_at() + interval);
    // Posts the timer event
    timer.async_wait(tick);

//    std::cout << "Tick" << std::endl;

    SerialReader::radioModule->doCycle();
}

SerialReader::SerialReader()
{
    radioModule = new RadioModule();

    timer.async_wait(tick);

    std::thread t([]()
                  { io_service.run(); });
    thread_.swap(t);

    // Enter IO loop. The timer will fire for the first time 1 second from now:
}
