#include <QCoreApplication>
#include <iostream>

#define DEBUG_SERIAL false

#include "SerialReader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

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
#endif

        if (port.portName().startsWith("tty") && port.manufacturer() == "Digi")
            targetPort = port;
    }

    if(targetPort.isNull())
    {
        qDebug("Couldn't find radio module");
        return 1;
    }

    SerialReader serialReader(targetPort, 921600);

    QString str = "Hello, World!";

    serialReader.send(0x0013A200423F474C, str.toUtf8(), str.length());


//    SerialReader reader();

    return QCoreApplication::exec();
}
