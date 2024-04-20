//
// Created by William Scheirey on 3/13/24.
//

#include "DataSimulator.h"

DataSimulator::DataSimulator(const QString& filePath, int dataIntervalMs, WebServer *webServer, QObject *parent): dataIntervalMs(dataIntervalMs), QObject(parent)
{
    file.setFileName(filePath);

    if(!file.open(QIODeviceBase::ReadOnly))
    {
        qDebug() << "Failed to open file: " << filePath;
    }

    _webServer = webServer;

    headers = nextLine();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DataSimulator::sendNextLine);

    timer->start(dataIntervalMs);
}

QList<QByteArray> DataSimulator::nextLine()
{
    QByteArray line = file.readLine();
    line.replace("\"", "");
    line.replace("\n", "");

    return line.split(',');
}

void DataSimulator::sendNextLine()
{
    if(!file.canReadLine())
    {
        file.close();
        file.open(QIODeviceBase::ReadOnly);
    }

    QString jsonString = "{";
    QList<QByteArray> values = nextLine();

    for(int i = 0; i < headers.length(); i++)
    {
        jsonString.append(QString::asprintf(R"("%s":"%s",)", headers[i].toStdString().c_str(), values[i].toStdString().c_str()));
    }
    jsonString.remove(jsonString.length() - 1, 1);
    jsonString.append("}");
    _webServer->broadcast(jsonString);
}

