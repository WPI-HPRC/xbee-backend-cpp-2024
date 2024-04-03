#include "SerialPort.h"

#include <iostream>

SerialPort::SerialPort()
{
    readQueue = circularQueueCreate<uint8_t>(65536);
    logFile = new QFile("/Users/will/Desktop/log.txt");

    logFile->open(QIODeviceBase::WriteOnly);
}

SerialPort::~SerialPort()
{
    stop();
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
    auto _ = port_->open(com_port_name, ec);
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

int SerialPort::write_some(const char *buf, const int &size)
{
    boost::system::error_code ec;

    if (!port_)
    { return -1; }
    if (size == 0)
    { return 0; }

    return (int) port_->write_some(boost::asio::buffer(buf, size), ec);
}

void SerialPort::async_read_some_()
{
    if (port_.get() == nullptr || !port_->is_open())
    {
        std::cout << "Port is not configured properly" << std::endl;
        return;
    }

    port_->async_read_some(
            boost::asio::buffer(read_buf_raw_, SERIAL_PORT_READ_BUF_SIZE),
            boost::bind(
                    &SerialPort::on_receive_,
                    this, boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void SerialPort::on_receive_(const boost::system::error_code &ec, size_t bytes_transferred)
{
    boost::mutex::scoped_lock look(mutex_);

    if (port_.get() == nullptr || !port_->is_open())
    {
        std::cout << "Port is not configured properly" << std::endl;
        return;
    }

    if (ec)
    {
        std::cout << "Error" << std::endl;
        async_read_some_();
        return;
    }

    unsigned int n = 0;
    for (unsigned int i = 0; i < bytes_transferred; ++i)
    {
        char c = read_buf_raw_[i];

        if (!(currentFrameBytesLeftToRead > 0 || c == XBee::StartDelimiter))
        {
            std::string str = QString::asprintf("%02x\n", c & 0xFF).toStdString();
            logFile->write(str.c_str(), (qint64) str.length());
        }

        if (currentFrameBytesLeftToRead > 0 || c == XBee::StartDelimiter)
        {
            if (currentFrameBytesLeftToRead <= 0)
            {
                currentFrameByteIndex = 0;
                currentFrameBytesLeftToRead = 3;
            }

            for (i; i < bytes_transferred && currentFrameBytesLeftToRead > 0; i++)
            {
                currentFrame[currentFrameByteIndex++] = (uint8_t) read_buf_raw_[i];

                std::string str = QString::asprintf("%02x ", read_buf_raw_[i] & 0xFF).toStdString();
                logFile->write(str.c_str(), (qint64) str.length());

                currentFrameBytesLeftToRead -= 1;

                if (currentFrameByteIndex == 3)
                {
                    currentFrameBytesLeftToRead = currentFrame[2] + 1;
                }
            }
        }

        if (currentFrameBytesLeftToRead == 0)
        {
            for (int j = 0; j < currentFrame[2] + 4; j++)
            {
                circularQueuePush(readQueue, currentFrame[j]);
            }
            packetsNotYetRead += 1;
            currentFrameBytesLeftToRead = -1;
            logFile->write("\n", 1);

            i -= 1; // Subtract 1 because i will be incremented at the end of this iteration of the for loop

//            std::string str = QString::asprintf("\n\nLAST BYTE: %02x\n", read_buf_raw_[i] & 0xFF).toStdString();
//            logFile->write(str.c_str(), (qint64) str.length());
        }
    }

    logFile->flush();
    async_read_some_();
}

void SerialPort::read(uint8_t *buffer, size_t length_bytes)
{
    if (packetsNotYetRead == 0)
    {
        return;
    }

    circularQueuePop(readQueue, buffer, length_bytes);
}