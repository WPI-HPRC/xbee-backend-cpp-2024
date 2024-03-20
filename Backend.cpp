//
// Created by William Scheirey on 3/13/24.
//

#include "Backend.h"
#include "DataSimulator.h"
#include <iostream>

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

Backend::Backend(QObject *parent) : QObject(parent)
{
    webServer = new WebServer(8001);

//    dataSimulator = new DataSimulator("/Users/will/Documents/GitHub/HPRC/telemetry-server/logs/2024-02-24_18.02.19_telemetry.csv", 25, webServer);
//    return;
    QSerialPortInfo targetPort = getTargetPort();

    if (targetPort.isNull())
    {
        qDebug("Couldn't find radio module");
        exit(1);
    }


    serialReader = new SerialReader(targetPort, QSerialPort::Baud115200);

    /*
    QString str = "Hello, World!";
    serialReader->radioModule->sendTransmitRequestCommand(0x0013A200423F474C,
                                                          (const uint8_t *) str.toStdString().c_str(), str.length());
                                                          */
    serialReader->radioModule->sendNodeDiscoveryCommand();
}
