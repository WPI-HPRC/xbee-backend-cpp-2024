//
// Created by William Scheirey on 3/20/24.
//

#include "RadioModule.h"
#include <iostream>

#include <QSerialPort>
#include <QSerialPortInfo>

#define DEBUG_SERIAL false

QSerialPortInfo getTargetPort()
{
    QList serialPorts = QSerialPortInfo::availablePorts();

    QSerialPortInfo targetPort;

#if DEBUG_SERIAL
    std::cout << "Available baud rates: \n";
    for(auto &baudRate : QSerialPortInfo::standardBaudRates())
    {
        std::cout << "\t" << baudRate << "\n";
    }

    std::cout << "\nFound serial ports: \n";
#endif

    for (auto &port: serialPorts)
    {
#if DEBUG_SERIAL
        std::cout << "\n" << port.portName().toStdString() << "\n";
        std::cout << "\tManufacturer: " << port.manufacturer().toStdString() << "\n";
        std::cout << "\tSystem location: " << port.systemLocation().toStdString() << "\n";
        std::cout << "\tSerial number: " << port.serialNumber().toStdString() << "\n";
        std::cout.flush();
#endif

        if (!port.portName().contains("cu.") && port.manufacturer().contains("Digi"))
        {
            targetPort = port;
        }
    }

    return targetPort;
}

RadioModule::RadioModule() : XBeeDevice()
{
    serialPort = new SerialPort();

    QSerialPortInfo targetPort = getTargetPort();

    if (targetPort.isNull())
    {
        qDebug() << "Couldn't find radio module";
        exit(1);
    }

    // yuck
    serialPort->start(getTargetPort().systemLocation().toStdString().c_str(), 115200);
//    std::cout << "Starting " << std::endl;

    webServer = new WebServer(8001);
}

void RadioModule::start()
{
    // Do nothing (only need this for SPI)
}

void RadioModule::writeBytes(const char *data, size_t length_bytes)
{
    std::cout << "Writing" << std::endl;
    serialPort->write_some(data, (int) length_bytes);
}

void RadioModule::packetRead()
{
    serialPort->packetsNotYetRead -= 1;
}

void RadioModule::readBytes(uint8_t *buffer, size_t length_bytes)
{
    serialPort->read(buffer, length_bytes);
}

void RadioModule::handleReceivePacket(XBee::ReceivePacket::Struct *frame)
{
//    return;
    webServer->dataReady(frame->data, frame->dataLength_bytes);
}

void RadioModule::handleReceivePacket64Bit(XBee::ReceivePacket64Bit::Struct *frame)
{
//    return;
//    std::cout << "RSSI: -" << std::dec << (int) (frame->negativeRssi & 0xFF) << "dbm\n";
    webServer->dataReady(frame->data, frame->dataLength_bytes);
}

void RadioModule::incorrectChecksum(uint8_t calculated, uint8_t received)
{
    std::string str = QString::asprintf("\nWRONG CHECKSUM. calculated: %02x, received: %02x\n", calculated & 0xFF,
                                        received & 0xFF).toStdString();
    serialPort->logFile->write(str.c_str(), (qint64) str.length());
}

