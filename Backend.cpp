//
// Created by William Scheirey on 3/13/24.
//

#include <qapplicationstatic.h>
#include "Backend.h"
#include "DataSimulator.h"
#include <iostream>

#define SIMULATE_DATA

QSerialPortInfo getTargetPort(QString portName)
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

        if (port.portName().contains(portName))
        {
            targetPort = port;
        }
    }

    return targetPort;
}

void Backend::flushFiles()
{
    for (RadioModule *radioModule: this->radioModules)
    {
        radioModule->dataLogger->flushDataFiles();
        radioModule->dataLogger->flushByteFile();
        radioModule->dataLogger->flushTextFile();
    }
}

Backend::Backend(QObject *parent) : QObject(parent)
{
#ifdef SIMULATE_DATA
    webServer = new WebServer(8001);

    dataSimulator = new DataSimulator(
            "/Users/will/Documents/MATLAB/DataRates/20_20/rocket.csv", 25, webServer);
#else

    webServer = new WebServer(8001);

    auto *groundModule = new ServingRadioModule(921600, new DataLogger(), webServer);
    radioModules.append(groundModule);

    timer = new QTimer();
    timer->setInterval(5);

    loopCount = 0;

    connect(timer, &QTimer::timeout, [this]()
            {
                for (auto radioModule: this->radioModules)
                {
                    radioModule->doCycle();
                }
            }
    );
    timer->start();
#endif
}
