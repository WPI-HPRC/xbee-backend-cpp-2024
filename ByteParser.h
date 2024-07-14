//
// Created by William Scheirey on 6/20/24.
//

#ifndef XBEE_BACKEND_CPP_BYTEPARSER_H
#define XBEE_BACKEND_CPP_BYTEPARSER_H

#include <QFile>

class ByteParser
{
public:
    ByteParser(const QString &fileIn);

    QByteArray arr;
};


#endif //XBEE_BACKEND_CPP_BYTEPARSER_H
