//
// Created by William Scheirey on 2/28/24.
//

#ifndef XBEE_BACKEND_CPP_SERIALREADER_H
#define XBEE_BACKEND_CPP_SERIALREADER_H

#include <QObject>
#include <QtSerialPort//QtSerialPort>


class SerialReader: public QObject
{
    Q_OBJECT

public:
    SerialReader(const QSerialPortInfo& port, int baudRate, QObject *parent = nullptr);
    void send(uint64_t address, const void *data, size_t size_bytes);

private:
    QSerialPort *m_serialPort;
    void connectSignals();

public slots:
    void    baudRateChanged(qint32 baudRate, QSerialPort::Directions directions);
    void	breakEnabledChanged(bool set);
    void	dataBitsChanged(QSerialPort::DataBits dataBits);
    void	dataTerminalReadyChanged(bool set);
    void	errorOccurred(QSerialPort::SerialPortError error);
    void	flowControlChanged(QSerialPort::FlowControl flow);
    void	parityChanged(QSerialPort::Parity parity);
    void	requestToSendChanged(bool set);
    void	stopBitsChanged(QSerialPort::StopBits stopBits);

    void	aboutToClose();
    void	bytesWritten(qint64 bytes);
    void	channelBytesWritten(int channel, qint64 bytes);
    void	channelReadyRead(int channel);
    void	readChannelFinished();
    void	readyRead();

};


#endif //XBEE_BACKEND_CPP_SERIALREADER_H
