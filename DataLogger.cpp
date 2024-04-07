//
// Created by William Scheirey on 2/28/24.
//

#include "DataLogger.h"
#include "QDateTime"
#include "Constants.h"
#include <QJsonDocument>

#define OFFICIAL_TEST false

DataLogger::DataLogger()
{
    createDirectory();
    createFile();
}

void DataLogger::createDirectory()
{
    logDir.setPath(Constants::LogDirPath);

    if (logDir.path().startsWith("~"))
    {
        logDir.setPath(logDir.path().replace("~", QDir::homePath()));
    }

    if (!logDir.exists())
    {
        if (logDir.mkpath("."))
        {
            qDebug("Created log directory");
        }
        else
        {
            qDebug("Failed to create log directory");
        }
    }
#if DEBUG_CSV
    else
    {
        qDebug("Log directory already exists");
    }
#endif
}

void DataLogger::createFile()
{
    QDateTime currentTime = QDateTime::currentDateTime();

    QString timeString = currentTime.toString(Constants::LogTimeFormat);

#if OFFICIAL_TEST
    rocketLogFile.open(logDir.path().append("/").append(timeString).append("_rocket.csv"));
    payloadLogFile.open(logDir.path().append("/").append(timeString).append("_payload.csv"));
#else
    rocketLogFile.open(logDir.path().append("/").append(timeString).append("_rocket_NOT_OFFICIAL.csv"));
    payloadLogFile.open(logDir.path().append("/").append(timeString).append("_payload_NOT_OFFICIAL.csv"));
#endif
}

void DataLogger::writeData(const QJsonObject &jsonData, DataLogger::PacketType packetType)
{
    switch (packetType)
    {
        case Rocket:
            rocketLogFile.write(jsonData);
            return;
        case Payload:
            payloadLogFile.write(jsonData);
            return;
        default:
            return;
    }
}

void DataLogger::dataReady(const char *data)
{
    writeData(QJsonDocument::fromJson(data).object(), DataLogger::Rocket);
}

void DataLogger::dataReady(const char *data, uint8_t rssi)
{
    QJsonObject json = QJsonDocument::fromJson(data).object();
    json.insert("rssi", -1 * (int) rssi);
    writeData(json, DataLogger::Rocket);
}