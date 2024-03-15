//
// Created by William Scheirey on 2/28/24.
//

#ifndef XBEE_BACKEND_CPP_SERIALREADER_H
#define XBEE_BACKEND_CPP_SERIALREADER_H

#include <QObject>
#include <QtSerialPort//QtSerialPort>

#include "XBeeDevice.h"

class SerialReader: public QObject
{
    Q_OBJECT

public:
    SerialReader(const QSerialPortInfo& port, int baudRate, QObject *parent = nullptr);
    XBeeDevice *radioModule;

private:
    QSerialPort *m_serialPort;
    QTimer *readTimer;
    void connectSignals();

public slots:
    void readyRead();

    static void baudRateChanged(qint32 baudRate, QSerialPort::Directions directions);
    static void	breakEnabledChanged(bool set);
    static void	dataBitsChanged(QSerialPort::DataBits dataBits);
    static void	dataTerminalReadyChanged(bool set);
    static void	errorOccurred(QSerialPort::SerialPortError error);
    static void	flowControlChanged(QSerialPort::FlowControl flow);
    static void	parityChanged(QSerialPort::Parity parity);
    static void	requestToSendChanged(bool set);
    static void	stopBitsChanged(QSerialPort::StopBits stopBits);

    static void	aboutToClose();
    static void	bytesWritten(qint64 bytes);
    static void	channelBytesWritten(int channel, qint64 bytes);
    static void	channelReadyRead(int channel);
    static void	readChannelFinished();

};


#endif //XBEE_BACKEND_CPP_SERIALREADER_H
