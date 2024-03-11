//
// Created by William Scheirey on 2/28/24.
//

#define connectHelper(method_signature) connect(m_serialPort, SIGNAL(method_signature), this, SLOT(method_signature))

#include <iostream>
#include "SerialReader.h"

SerialReader::SerialReader(const QSerialPortInfo& port, int baudRate, QObject *parent): QObject(parent)
{
    m_serialPort = new QSerialPort();

    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setPort(port);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if(m_serialPort->open(QIODevice::ReadWrite))
    {
        std::cout << "Opened serial port " << port.manufacturer().toStdString() << " - " << port.portName().toStdString() << " at baud rate " << baudRate << "\n";
        receivePacket = new char[MAX_PACKET_LENGTH];
        sendPacket = new uint8_t[MAX_PACKET_LENGTH];
        connectSignals();
    }
    else
    {
        qWarning() << "Couldn't open serial port " << port.portName();
    }

}

void SerialReader::send(uint64_t address, const void *data, size_t size_bytes)
{
    size_t contentLength = size_bytes + 14; // +4 for start delimiter, length, and checksum, +8 for address

    auto *packet = sendPacket;

    size_t index = 0;

    packet[index++] = 0x7E; // Start delimiter

    packet[index++] = (contentLength >> 8) & 0xFF; // Length high byte
    packet[index++] = contentLength & 0xFF;        // Length low byte

    packet[index++] = 0x10; // Frame type
    packet[index++] = 0x01; // Frame ID

    for (int i = 0; i < 8; i++)
    {
        packet[index++] = (address >> ((7 - i) * 8)) & 0xFF;
    }

    packet[index++] = 0xFF; // Reserved
    packet[index++] = 0xFE; // Reserved

    packet[index++] = 0x00; // Broadcast radius

    packet[index++] = 0x00; // Options byte

    memcpy(&packet[index++], data, size_bytes);

    size_t checksum_temp = 0;

    for (size_t i = 3; i < index; i++)
    {
        checksum_temp += packet[i];
    }

    uint8_t checksum = checksum_temp & 0xFF;

    checksum = 0xFF - checksum;

    packet[contentLength - 1] = checksum;

    m_serialPort->write(QByteArray::fromRawData((const char*)packet, (long long)contentLength));
}

void SerialReader::connectSignals()
{
    connectHelper(baudRateChanged(qint32,QSerialPort::Directions));
    connectHelper(breakEnabledChanged(bool));
    connectHelper(dataBitsChanged(QSerialPort::DataBits));
    connectHelper(dataTerminalReadyChanged(bool));
    connectHelper(errorOccurred(QSerialPort::SerialPortError));
    connectHelper(flowControlChanged(QSerialPort::FlowControl));
    connectHelper(parityChanged(QSerialPort::Parity));
    connectHelper(requestToSendChanged(bool));
    connectHelper(stopBitsChanged(QSerialPort::StopBits));
    connectHelper(aboutToClose());
    connectHelper(bytesWritten(qint64));
    connectHelper(channelBytesWritten(int,qint64));
    connectHelper(channelReadyRead(int));
    connectHelper(readChannelFinished());
    connectHelper(readyRead());
}

void SerialReader::receive(const uint8_t *packet)
{

}

void    SerialReader::baudRateChanged(qint32 baudRate, QSerialPort::Directions directions)
{
    qDebug() << "Baud rate changed to: " << baudRate << " direction: " << directions;
}
void	SerialReader::breakEnabledChanged(bool set)
{
    qDebug() << "Break enabled changed to: " << (set ? "TRUE" : "FALSE");
}
void	SerialReader::dataBitsChanged(QSerialPort::DataBits dataBits)
{
    qDebug() << "Data bits changed to: " << dataBits;
}
void	SerialReader::dataTerminalReadyChanged(bool set)
{
    qDebug() << "Data terminal ready changed to: " << (set ? "TRUE" : "FALSE");

}
void	SerialReader::errorOccurred(QSerialPort::SerialPortError error)
{
    qDebug() << "Serial error occurred: " << error;
}
void	SerialReader::flowControlChanged(QSerialPort::FlowControl flow)
{
    qDebug() << "Flow control changed to: " << flow;
}
void	SerialReader::parityChanged(QSerialPort::Parity parity)
{
    qDebug() << "Parity changed to: " << parity;
}
void	SerialReader::requestToSendChanged(bool set)
{
    qDebug() << "Request to send changed to: " << (set ? "TRUE" : "FALSE");
}
void	SerialReader::stopBitsChanged(QSerialPort::StopBits stopBits)
{
    qDebug() << "Stop bits changed to: " << stopBits;
}

void	SerialReader::aboutToClose()
{
    qDebug() << "About to close";
}

void	SerialReader::bytesWritten(qint64 bytes)
{
    qDebug() << "Bytes written: " << bytes;
}

void	SerialReader::channelBytesWritten(int channel, qint64 bytes)
{
    qDebug() << "Channel bytes written. Channel: " << channel << ", bytes: " << bytes;
}

void	SerialReader::channelReadyRead(int channel)
{
    qDebug() << "Channel ready read: " << channel;
}

void	SerialReader::readChannelFinished()
{
    qDebug() << "Read channel finished";
}

void	SerialReader::readyRead()
{
    qDebug() << "Ready read";

    m_serialPort->read(receivePacket, MAX_PACKET_LENGTH);

}

