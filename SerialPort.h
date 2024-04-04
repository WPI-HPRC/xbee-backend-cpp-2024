#pragma once

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>
#include "xbee/XBeeUtility.h"
#include "QFile"

#include "xbee/circularQueue.hpp"

typedef boost::shared_ptr<boost::asio::serial_port> serial_port_ptr;

#define SERIAL_PORT_READ_BUF_SIZE 65536

class SerialPort
{
protected:
    serial_port_ptr port_;
    boost::mutex mutex_;

    char read_buf_raw_[SERIAL_PORT_READ_BUF_SIZE];

private:
    SerialPort(const SerialPort &p);

    SerialPort &operator=(const SerialPort &p);

    std::thread thread_;

    CircularQueue<uint8_t> *readQueue;

    boost::asio::io_service io_service_;

    uint8_t currentFrame[XBee::MaxFrameBytes];

    uint8_t currentFrameByteIndex = 0;

public:

    SerialPort();

    virtual ~SerialPort();

    bool start(const char *com_port_name, int baud_rate);

    void stop();

    int write_some(const char *buf, const int &size);

    void read(uint8_t *buffer, size_t length_bytes);

    int packetsNotYetRead = 0;

    QFile *logFile;

    int currentFrameBytesLeftToRead = -1;
protected:
    virtual void async_read_some_();

    virtual void on_receive_(const boost::system::error_code &ec, size_t bytes_transferred);

};