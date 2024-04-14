//
// Created by William Scheirey on 3/20/24.
//

#include "RadioModule.h"
#include <iostream>

#include <format>

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
    QSerialPortInfo targetPort = getTargetPort();

    if (targetPort.isNull())
    {
        qDebug() << "Couldn't find radio module";
        exit(1);
    }

    webServer = new WebServer(8001);

    serialPort = new SerialPort(targetPort, QSerialPort::Baud115200, &webServer->dataLogger);


    sendTransmitRequestsImmediately = true;

    sendFramesImmediately = true;

    logWrongChecksums = false;

//    queryParameterRemote(0x0013a200422cdf59, XBee::AtCommand::SupplyVoltage);

//    queryParameter(XBee::AtCommand::ChannelMask);
//    queryParameter(XBee::AtCommand::MinimumFrequencies);

//    setParameter(XBee::AtCommand::ApiOptions, 0x02);
//    setParameterRemote(, XBee::AtCommand::PowerLevel, 0x02);
//    queryParameterRemote(0x0013A20042378B08, XBee::AtCommand::PowerLevel);
//    sendNodeDiscoveryCommand();
}

void RadioModule::start()
{
    // Do nothing (only need this for SPI)
}

void RadioModule::writeBytes(const char *data, size_t length_bytes)
{
    int bytes_written = serialPort->write(data, (int) length_bytes);

    webServer->dataLogger.writeToTextFile("Writing: ");
    for (int i = 0; i < length_bytes; i++)
    {
        webServer->dataLogger.writeToTextFile(QString::asprintf("%02x ", data[i] & 0xFF));
    }
    webServer->dataLogger.writeToTextFile("\n");
    webServer->dataLogger.flushTextFile();


    if (bytes_written != length_bytes)
    {
        log("FAILED TO WRITE ALL BYTES. EXPECTED %d, RECEIVED %d", length_bytes, bytes_written);
    }
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
    webServer->dataReady(frame->data, frame->dataLength_bytes);
}

void RadioModule::handleReceivePacket64Bit(XBee::ReceivePacket64Bit::Struct *frame)
{
    std::cout << "RSSI: -" << std::dec << (int) (frame->negativeRssi & 0xFF) << "dbm\n";
    webServer->dataReady(frame->data, frame->dataLength_bytes, frame->negativeRssi);
}

void RadioModule::incorrectChecksum(uint8_t calculated, uint8_t received)
{
    std::string str = QString::asprintf("\nWRONG CHECKSUM. calculated: %02x, received: %02x\n\n", calculated & 0xFF,
                                        received & 0xFF).toStdString();
    webServer->dataLogger.writeToByteFile(str.c_str(), str.length());
    webServer->dataLogger.writeToTextFile(str.c_str(), str.length());

    webServer->dataLogger.flushByteFile();
    webServer->dataLogger.flushTextFile();

    serialPort->logFile->write(str.c_str(), (qint64) str.length());
}

void RadioModule::log(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vprintf(format, args);

    char buff[256];
    vsnprintf(buff, sizeof(buff), format, args);
    std::string str = buff;

    webServer->dataLogger.writeToTextFile(str.c_str(), str.length());

    webServer->dataLogger.flushTextFile();

    va_end(args);
}

void RadioModule::didCycle()
{
//    return;
    if (cycleCount % 500 == 0)
    {

        queryParameterRemote(0x0013A20042378B08, XBee::AtCommand::UnicastAttemptedCount);
        queryParameterRemote(0x0013A20042378B08, XBee::AtCommand::TransmissionFailureCount);
        queryParameterRemote(0x0013A20042378B08, XBee::AtCommand::MacAckFailureCount);

    }

    cycleCount++;
}

void RadioModule::_handleRemoteAtCommandResponse(const uint8_t *frame, uint8_t length_bytes, bool paramWasBeingWaitedOn)
{
    uint16_t command = getRemoteAtCommand(frame);

    uint64_t address = getAddress(&frame[XBee::RemoteAtCommandResponse::BytesBeforeAddress]);

    log("Remote AT response from %016llx: ", (unsigned long long) address);
    if (command == XBee::AtCommand::SupplyVoltage)
    {
        uint16_t voltage = frame[XBee::RemoteAtCommandResponse::BytesBeforeCommandData] << 8 |
                           frame[XBee::RemoteAtCommandResponse::BytesBeforeCommandData + 1];

        log("voltage = %f mV", (float) voltage / 1000);
    }

    log("%c%c: ", (command & 0xFF00) >> 8, command & 0x00FF);
    for (uint8_t i = 0; i < length_bytes - XBee::RemoteAtCommandResponse::PacketBytes; i++)
    {
        log("%02x ", (int) (frame[XBee::RemoteAtCommandResponse::BytesBeforeCommandData + i] & 0xFF));
    }

    log("\n");
}

