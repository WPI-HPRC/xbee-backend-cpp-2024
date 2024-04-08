//
// Created by William Scheirey on 2/28/24.
//

#include "DataLogger.h"
#include "QDateTime"
#include "Constants.h"
#include <QJsonDocument>

#define OFFICIAL_TEST true

DataLogger::DataLogger()
{
    createFiles();
}

void DataLogger::createDirectory(const QString &timeString)
{
    logDir.setPath(Constants::LogDirPath);
    logDir.setPath(logDir.path().append("/").append(timeString));

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

void DataLogger::createFiles()
{
    QDateTime currentTime = QDateTime::currentDateTime();

    QString timeString = currentTime.toString(Constants::LogTimeFormat);

    createDirectory(timeString);

#if OFFICIAL_TEST
    rocketLogFile.open(logDir.path().append("/").append(timeString).append("_rocket.csv"));
    payloadLogFile.open(logDir.path().append("/").append(timeString).append("_payload.csv"));
    byteLog.setFileName(logDir.path().append("/").append(timeString).append("_bytes.txt"));
    byteLog.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text);
    textLog.setFileName(logDir.path().append("/").append(timeString).append("_log.txt"));
    textLog.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text);
#else
    rocketLogFile.open(logDir.path().append("/").append(timeString).append("_rocket_NOT_OFFICIAL.csv"));
    payloadLogFile.open(logDir.path().append("/").append(timeString).append("_payload_NOT_OFFICIAL.csv"));
    byteLog.setFileName(logDir.path().append("/").append(timeString).append("_bytes_NOT_OFFICIAL.txt"));
    byteLog.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text);
    textLog.setFileName(logDir.path().append("/").append(timeString).append("_log_NOT_OFFICIAL.txt"));
    textLog.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text);
#endif
}

void DataLogger::writeToByteFile(const char *text, size_t size)
{
    byteLog.write(text, size);
}

void DataLogger::writeToByteFile(const QString &str)
{
    writeToByteFile(str.toStdString().c_str(), str.toStdString().length());
}


void DataLogger::flushByteFile()
{
    byteLog.flush();
}

void DataLogger::writeToTextFile(const char *text, size_t size)
{
    textLog.write(text, size);
}

void DataLogger::writeToTextFile(const QString &str)
{
    writeToTextFile(str.toStdString().c_str(), str.toStdString().length());
}


void DataLogger::flushTextFile()
{
    textLog.flush();
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