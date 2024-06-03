//
// Created by William Scheirey on 3/20/24.
//

#include "RadioModule.h"
#include <iostream>
#include <regex>

#include <QSerialPortInfo>

QSerialPortInfo getTargetPort()
{
    QList serialPorts = QSerialPortInfo::availablePorts();

    QSerialPortInfo targetPort;

#ifdef DEBUG_SERIAL
    std::cout << "Available baud rates: \n";
    for (auto &baudRate: QSerialPortInfo::standardBaudRates())
    {
        std::cout << "\t" << baudRate << "\n";
    }

    std::cout << "\nFound serial ports: \n";
#endif

    for (auto &port: serialPorts)
    {
#ifdef DEBUG_SERIAL
        std::cout << "\n" << port.portName().toStdString() << "\n";
        std::cout << "\tManufacturer: " << port.manufacturer().toStdString() << "\n";
        std::cout << "\tSystem location: " << port.systemLocation().toStdString() << "\n";
        std::cout << "\tSerial number: " << port.serialNumber().toStdString() << "\n";
        std::cout.flush();
#endif

        if (!port.portName().contains("cu.") && port.manufacturer().contains("Digi") ||
            port.manufacturer().contains("FTDI"))
        {
            targetPort = port;
        }
    }

    return targetPort;
}

DataLogger::Packet parsePacket(const uint8_t *frame)
{
    std::string str;

    // This way of assigning the packet type seems redundant, but the packetType byte can take on any value from 0-255; we want to set it to an enum value that we understand
    DataLogger::PacketType packetType;

    switch (frame[0])
    {
        case DataLogger::Rocket:
            str = JS::serializeStruct(*(RocketTelemPacket *) (&frame[1]));
            packetType = DataLogger::Rocket;
            break;
        case DataLogger::Payload:
            str = JS::serializeStruct(*(PayloadTelemPacket *) (&frame[1]));
            packetType = DataLogger::Payload;
            break;
        default:
            str = "";
            packetType = DataLogger::Unknown;
            break;
    }

    str = std::regex_replace(str, std::regex("nan"), "0");
    str = std::regex_replace(str, std::regex("inf"), "0");

    return {str, packetType};
}

void RadioModule::configureRadio()
{
    std::cout << "Configuring radio" << std::endl; // Does nothing right now
//    setParameter(XBee::AtCommand::ApiOptions, 0x02);
//    setParameterRemote(, XBee::AtCommand::PowerLevel, 0x02);
}

RadioModule::RadioModule(int baudRate, DataLogger *logger, const QSerialPortInfo &portInfo) : XBeeDevice()
{
    dataLogger = logger;

    serialPort = new SerialPort(portInfo, baudRate, dataLogger,
                                XBee::ApiOptions::ApiWithoutEscapes);

    sendTransmitRequestsImmediately = true;

    sendFramesImmediately = true;

    logWrongChecksums = true;

    configureRadio();
}

RadioModule::RadioModule(int baudRate, DataLogger *logger) : XBeeDevice()
{
    QSerialPortInfo targetPort = getTargetPort();

#ifdef REQUIRE_XBEE_MODULE
    if (targetPort.isNull())
    {
        qDebug() << "Couldn't find radio module";
        exit(1);
    }
#endif

    *this = RadioModule(baudRate, logger, targetPort);
}

void RadioModule::start()
{
    // Do nothing (only need this for SPI)
}

void RadioModule::writeBytes(const char *data, size_t length_bytes)
{
    if (length_bytes < 0)
        return;
#ifndef REQUIRE_XBEE_MODULE
    if(!serialPort->isOpen())
    {
        return;
    }
#endif
    int bytes_written = serialPort->write(data, (int) length_bytes);

    dataLogger->writeToTextFile("Writing: ");
    for (int i = 0; i < length_bytes; i++)
    {
        dataLogger->writeToTextFile(QString::asprintf("%02x ", data[i] & 0xFF));
    }
    dataLogger->writeToTextFile("\n");
    dataLogger->flushTextFile();


    if (bytes_written != length_bytes)
    {
        log("FAILED TO WRITE ALL BYTES. EXPECTED %d, RECEIVED %d\n", length_bytes, bytes_written);
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
    std::cout << "Received packet" << std::endl;
    lastPacket = parsePacket(frame->data);
    dataLogger->dataReady(lastPacket.data.c_str(), lastPacket.packetType);
}

void RadioModule::handleReceivePacket64Bit(XBee::ReceivePacket64Bit::Struct *frame)
{
    lastPacket = parsePacket(frame->data);
    dataLogger->dataReady(lastPacket.data.c_str(), lastPacket.packetType, frame->negativeRssi);
}

void RadioModule::incorrectChecksum(uint8_t calculated, uint8_t received)
{
    std::string str = QString::asprintf("\nWRONG CHECKSUM. calculated: %02x, received: %02x\n\n", calculated & 0xFF,
                                        received & 0xFF).toStdString();

    log(str.c_str());

    dataLogger->writeToByteFile(str.c_str(), str.length());
    dataLogger->writeToTextFile(str.c_str(), str.length());

    dataLogger->flushByteFile();
    dataLogger->flushTextFile();
}

void RadioModule::log(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vprintf(format, args);
    fflush(stdout);

    char buff[256];
    vsnprintf(buff, sizeof(buff), format, args);
    std::string str = buff;

    dataLogger->writeToTextFile(str.c_str(), str.length());

    dataLogger->flushTextFile();

    va_end(args);
}

void RadioModule::didCycle()
{
    return;
    if (cycleCount % 5 == 0)
    {
        dummyPacket.timestamp = cycleCount / 5;
        sendTransmitRequestCommand(0x0013A200423F474C, (uint8_t *) &dummyPacket, sizeof(dummyPacket));

//        std::string str = "Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!";
//        queryParameter(XBee::AtCommand::Temperature);
//        queryParameterRemote(0x0013A200422CDAC2, XBee::AtCommand::Temperature);
//        queryParameterRemote(0x0013A200422CDAC4, XBee::AtCommand::Temperature);


//        queryParameterRemote(0x0013A200422CDF59, XBee::AtCommand::UnicastAttemptedCount);
//        queryParameterRemote(0x0013A200422CDF59, XBee::AtCommand::TransmissionFailureCount);
//        queryParameterRemote(0x0013A200422CDF59, XBee::AtCommand::MacAckFailureCount);

//        queryParameterRemote(0x0013A2FE643CA484, XBee::AtCommand::Temperature);

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

        log("voltage = %f mV\n", (float) voltage / 1000);
        return;
    }

    log("%c%c: ", (command & 0xFF00) >> 8, command & 0x00FF);
    for (uint8_t i = 0; i < length_bytes - XBee::RemoteAtCommandResponse::PacketBytes; i++)
    {
        log("%02x ", (int) (frame[XBee::RemoteAtCommandResponse::BytesBeforeCommandData + i] & 0xFF));
    }

    log("\n");
}

ServingRadioModule::ServingRadioModule(int baudRate, DataLogger *logger, const QSerialPortInfo &portInfo, WebServer *server): RadioModule(baudRate, logger, portInfo)
{
    webServer = server;
}

ServingRadioModule::ServingRadioModule(int baudRate, DataLogger *logger, WebServer *server): RadioModule(baudRate, logger)
{
    webServer = server;
}

void ServingRadioModule::handleReceivePacket64Bit(XBee::ReceivePacket64Bit::Struct *frame)
{
    log("RSSI: -%ddbm\n", frame->negativeRssi);
    RadioModule::handleReceivePacket64Bit(frame);

//    webServer->broadcast(QString::fromStdString(lastPacket.data));
}

void ServingRadioModule::handleReceivePacket(XBee::ReceivePacket::Struct *frame)
{
    RadioModule::handleReceivePacket(frame);

//    webServer->broadcast(QString::fromStdString(lastPacket.data));
}

void RocketTestModule::didCycle()
{
    if (cycleCount % 5 == 0)
    {
        packet.timestamp = cycleCount / 5;
        packet.epochTime = QDateTime::currentMSecsSinceEpoch();

        sendTransmitRequestCommand(GROUND_STATION_ADDR, (uint8_t *)&packet, sizeof(packet));
    }
    cycleCount++;
}

void PayloadTestModule::didCycle()
{
    if (cycleCount % 5 == 0)
    {
        packet.timestamp = cycleCount / 5;
        packet.epochTime = QDateTime::currentMSecsSinceEpoch();

        sendTransmitRequestCommand(GROUND_STATION_ADDR, (uint8_t *)&packet, sizeof(packet));
    }
    cycleCount++;
}


