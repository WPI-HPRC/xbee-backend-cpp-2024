#include "SerialPort.h"

#include <iostream>

SerialPort::SerialPort(void) : end_of_line_char_('\n')
{
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
    port_->open(std::string("/dev/tty.usbserial-A28DMVHS"), ec);

    // option settings...
    port_->set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    port_->set_option(boost::asio::serial_port_base::character_size(8));
    port_->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    port_->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    port_->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

//    boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service_));

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

void SerialPort::read_some(const char *buffer, size_t size_bytes)
{
    if (port_.get() == nullptr || !port_->is_open())
    { return; }


    port_->read_some(
            boost::asio::buffer(read_buf_raw_, size_bytes)
    );
}