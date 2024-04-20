#pragma once

#include "XBee/XBeeUtility.h"
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QDebug>

#include "DataLogger.h"

#include "XBee/circularQueue.hpp"

#define SERIAL_PORT_READ_BUF_SIZE 65536

class SerialPort : public QObject
{
Q_OBJECT

private:
    QSerialPort *m_serialPort;

    CircularQueue<uint8_t> *readQueue;

    char readBuffer[SERIAL_PORT_READ_BUF_SIZE];

    uint8_t currentFrame[XBee::MaxFrameBytes];

    uint8_t currentFrameByteIndex = 0;

    void connectSignals();

    DataLogger *dataLogger;

public:

    SerialPort(QSerialPortInfo port, QSerialPort::BaudRate baudRate, DataLogger *dataLogger);

    int write(const char *buf, const int &size);

    void read(uint8_t *buffer, size_t length_bytes);

    int packetsNotYetRead = 0;

    QFile *logFile;

    int currentFrameBytesLeftToRead = -1;

public slots:

    void readyRead();

    static void baudRateChanged(qint32 baudRate, QSerialPort::Directions directions);

    static void breakEnabledChanged(bool set);

    static void dataBitsChanged(QSerialPort::DataBits dataBits);

    static void dataTerminalReadyChanged(bool set);

    static void errorOccurred(QSerialPort::SerialPortError error);

    static void flowControlChanged(QSerialPort::FlowControl flow);

    static void parityChanged(QSerialPort::Parity parity);

    static void requestToSendChanged(bool set);

    static void stopBitsChanged(QSerialPort::StopBits stopBits);

    static void aboutToClose();

    void bytesWritten(qint64 bytes);

    static void channelBytesWritten(int channel, qint64 bytes);

    static void channelReadyRead(int channel);

    static void readChannelFinished();
};
