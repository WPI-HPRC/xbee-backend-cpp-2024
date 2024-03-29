#include "SerialPort.h"

#include <iostream>

SerialPort::SerialPort(void) : end_of_line_char_('\n')
{
    readQueue = circularQueueCreate<uint8_t>(65536);
}

SerialPort::~SerialPort(void)
{
    stop();
}

char SerialPort::end_of_line_char() const
{
    return this->end_of_line_char_;
}

void SerialPort::end_of_line_char(const char &c)
{
    this->end_of_line_char_ = c;
}

bool SerialPort::start(const char *com_port_name, int baud_rate)
{
    boost::system::error_code ec;

    if (port_)
    {
        std::cout << "error : port is already opened..." << std::endl;
        return false;
    }

    port_ = serial_port_ptr(new boost::asio::serial_port(io_service_));
    port_->open(com_port_name, ec);
    if (ec)
    {
        std::cout << "error : port_->open() failed...com_port_name="
                  << com_port_name << ", e=" << ec.message().c_str() << std::endl;
        return false;
    }

    // option settings...
    port_->set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    port_->set_option(boost::asio::serial_port_base::character_size(8));
    port_->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    port_->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    port_->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

    async_read_some_();
    std::thread t([this]()
                  { this->io_service_.run(); });
    thread_.swap(t);

    return true;
}


void SerialPort::stop()
{
    boost::mutex::scoped_lock look(mutex_);

    if (port_)
    {
        port_->cancel();
        port_->close();
        port_.reset();
    }

    if (thread_.joinable())
    {
        thread_.join();
    }

    io_service_.stop();
    io_service_.reset();
}

int SerialPort::write_some(const std::string &buf)
{
    return write_some(buf.c_str(), buf.size());
}

int SerialPort::write_some(const char *buf, const int &size)
{
    boost::system::error_code ec;

    if (!port_)
    { return -1; }
    if (size == 0)
    { return 0; }

    return port_->write_some(boost::asio::buffer(buf, size), ec);
}

void SerialPort::async_read_some_()
{
    if (port_.get() == nullptr || !port_->is_open())
    {
        std::cout << "Not getting" << std::endl;
        return;
    }

//    std::cout << "Reading" << std::endl;

    port_->async_read_some(
            boost::asio::buffer(read_buf_raw_, SERIAL_PORT_READ_BUF_SIZE),
            boost::bind(
                    &SerialPort::on_receive_,
                    this, boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void SerialPort::on_receive_(const boost::system::error_code &ec, size_t bytes_transferred)
{
//    std::cout << "Packets not yet read: " << packetsNotYetRead << std::endl;
    boost::mutex::scoped_lock look(mutex_);

    if (port_.get() == NULL || !port_->is_open())
    {
        std::cout << "Not reading" << std::endl;
        return;
    }
    if (ec)
    {
        std::cout << "Error" << std::endl;
        async_read_some_();
        return;
    }

//    std::cout << "Bytes transferred: " << std::dec << (int) bytes_transferred << std::endl;
//    readQueue.push((uint8_t) bytes_transferred);

    unsigned int n = 0;
    for (unsigned int i = 0; i < bytes_transferred; ++i)
    {
        char c = read_buf_raw_[i];

        if (currentFrameBytesLeftToRead > 0 || c == XBee::StartDelimiter)
        {
            if (currentFrameBytesLeftToRead <= 0)
            {
                currentFrameByteIndex = 0;
                currentFrameBytesLeftToRead = 3;
//                std::cout << "Start delimiter" << std::endl;
            }
            else
            {
//                std::cout << "Already reading packet. Bytes left: " << std::dec << (int) currentFrameBytesLeftToRead
//                          << std::endl;
            }
            for (n = i; n < bytes_transferred && currentFrameBytesLeftToRead > 0; n++)
            {
//                std::cout << "Current frame byte index: " << std::dec << (int) currentFrameByteIndex
//                          << ", bytes left to read: " << (int) currentFrameBytesLeftToRead << std::endl;
//
//                std::cout << "Char: " << std::hex << (int) (read_buf_raw_[n] & 0xFF) << std::endl;

                currentFrame[currentFrameByteIndex++] = (uint8_t) read_buf_raw_[n];

                currentFrameBytesLeftToRead -= 1;

                if (currentFrameByteIndex == 3)
                {
                    currentFrameBytesLeftToRead = currentFrame[2] + 1;
//                    std::cout << "Length: " << std::dec << (int) currentFrame[2] << std::endl;
                }
            }
            i = n;
        }
        else
        {
//            std::cout << "No start delimiter: " << std::hex << (int) (c & 0xFF) << std::endl;
        }
        if (currentFrameBytesLeftToRead == 0)
        {
            for (int j = 0; j < currentFrame[2] + 4; j++)
            {
//                std::cout << "Adding " << std::hex << (int) (currentFrame[j] & 0xFF) << std::endl;
                circularQueueAdd(readQueue, currentFrame[j]);
            }
//            std::cout << "Added packet to readQueue" << std::endl;
            packetsNotYetRead += 1;
            currentFrameBytesLeftToRead = -1;
        }
    }
    async_read_some_();
}

void SerialPort::read(uint8_t *buffer, size_t length_bytes)
{
    if (packetsNotYetRead == 0)
    {
//        std::cout << "Not reading" << std::endl;
        return;
    }

//    std::cout << "Trying to read " << (int) length_bytes << " bytes" << std::endl;
//    std::cout << "Length"

    circularQueueRead(readQueue, buffer, length_bytes);
//    packetsNotYetRead -= 1;

}