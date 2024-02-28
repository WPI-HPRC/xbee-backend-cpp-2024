#include <QCoreApplication>
#include <QDebug>
#include <iostream>

#include "SerialReader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QList serialPorts = QSerialPortInfo::availablePorts();

    QSerialPortInfo targetPort;

    std::cout << "Available baud rates: \n";
    for(auto &baudRate : QSerialPortInfo::standardBaudRates())
    {
        std::cout << "\t" << baudRate << "\n";
    }

    std::cout << "\nFound serial ports: \n";
    for (auto &port: serialPorts)
    {
        std::cout << "\n" << port.portName().toStdString() << "\n";
        std::cout << "\tManufacturer: " << port.manufacturer().toStdString() << "\n";
        std::cout << "\tSystem location: " << port.systemLocation().toStdString() << "\n";
        std::cout << "\tSerial number: " << port.serialNumber().toStdString() << "\n";

        if (!port.portName().startsWith("tty") || port.portName().contains("Bluetooth"))
            continue;
    }

    std::cout.flush();



    SerialReader serialReader(serialPorts.at(1), 9600);


//    SerialReader reader();

    return QCoreApplication::exec();
}
