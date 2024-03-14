//
// Created by William Scheirey on 2/28/24.
//

#define connectHelper(method_signature) connect(m_serialPort, SIGNAL(method_signature), this, SLOT(method_signature))

#include <iostream>
#include "SerialReader.h"

#define DEBUG false

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
        connectSignals();
    }
    else
    {
        qWarning() << "Couldn't open serial port " << port.portName();
        return;
    }

    std::cout.flush();

    radioModule = new XBeeDevice(m_serialPort);

}

void SerialReader::connectSignals()
{
#if DEBUG
    connectHelper(baudRateChanged(qint32,QSerialPort::Directions));
    connectHelper(breakEnabledChanged(bool));
    connectHelper(dataBitsChanged(QSerialPort::DataBits));
    connectHelper(dataTerminalReadyChanged(bool));
    connectHelper(errorOccurred(QSerialPort::SerialPortError));
    connectHelper(flowControlChanged(QSerialPort::FlowControl));
    connectHelper(parityChanged(QSerialPort::Parity));
    connectHelper(requestToSendChanged(bool));
    connectHelper(stopBitsChanged(QSerialPort::StopBits));
#endif
    connectHelper(aboutToClose());
    connectHelper(bytesWritten(qint64));
    connectHelper(channelBytesWritten(int,qint64));
    connectHelper(channelReadyRead(int));
    connectHelper(readChannelFinished());
    connectHelper(readyRead());
}

void SerialReader::baudRateChanged(qint32 baudRate, QSerialPort::Directions directions)
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
#if DEBUG
    qDebug() << "About to close";
#endif
}

void	SerialReader::bytesWritten(qint64 bytes)
{
#if DEBUG
    qDebug() << "Bytes written: " << bytes;
#endif
}

void	SerialReader::channelBytesWritten(int channel, qint64 bytes)
{
#if DEBUG
    qDebug() << "Channel bytes written. Channel: " << channel << ", bytes: " << bytes;
#endif
}

void	SerialReader::channelReadyRead(int channel)
{
#if DEBUG
    qDebug() << "Channel ready read: " << channel;
#endif
}

void	SerialReader::readChannelFinished()
{
#if DEBUG
    qDebug() << "Read channel finished";
#endif
}

void	SerialReader::readyRead()
{
#if DEBUG
    qDebug() << "Ready read";
#endif

    radioModule->receive();
}

