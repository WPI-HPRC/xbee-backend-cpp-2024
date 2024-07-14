//
// Created by William Scheirey on 6/20/24.
//

#include "ByteParser.h"
#include <Qt>
#include <QRegularExpression>
#include <iostream>
#include "Utility.h"
#include "XBee/XBeeUtility.h"
#include "DataLogger.h"
#include "json_struct.h"


QByteArray hexToBytes(const QString &hexString)
{
    QByteArray rawBytes;
    QString cleanHexString = hexString;
    cleanHexString.remove(QRegularExpression("\\s")); // Remove all spaces and newlines
    rawBytes = QByteArray::fromHex(cleanHexString.toLatin1());
    return rawBytes;
}

// Function to read hex string from file and convert to raw bytes
QByteArray hexToBytesFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Could not open file:" << filePath;
        return QByteArray();
    }

    QTextStream in(&file);
    QString hexString = in.readAll();
    file.close();

    return hexToBytes(hexString);
}

ByteParser::ByteParser(const QString &fileIn)
{
    arr = hexToBytesFromFile(fileIn);

    DataLogger dataLogger;

    for(int i = 0; i < 37; i++)
    {
        int startIndex = i * (XBee::ReceivePacket::FrameBytes + sizeof(RocketTelemPacket) + 1);
        auto *raw = (uint8_t *)(&(arr.data()[startIndex + XBee::ReceivePacket::BytesBeforePayload + 1]));

        std::string str = JS::serializeStruct(*(RocketTelemPacket *)(raw));

        std::cout << str << std::endl;

        dataLogger.dataReady(str.c_str(), DataLogger::Rocket);
/*
        int startIndex = i * (XBee::ReceivePacket::FrameBytes + sizeof(RocketTelemPacket) + 1);
        auto *raw = (uint8_t *)(&(arr.data()[startIndex + XBee::ReceivePacket::BytesBeforePayload + 1]));

        auto *packet = (RocketTelemPacket *) raw;

        std::cout << "Timestamp: " << packet->timestamp << std::endl;
        */
    }

    dataLogger.flushDataFiles();
}