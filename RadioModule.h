//
// Created by William Scheirey on 3/20/24.
//

#ifndef XBEE_BACKEND_CPP_RADIOMODULE_H
#define XBEE_BACKEND_CPP_RADIOMODULE_H

#include "XBee/XBeeDevice.h"
#include "SerialPort.h"

class RadioModule : public XBeeDevice
{
public:
    RadioModule();

    SerialPort serialPort;

    void serialRead(const char *buffer, size_t length_bytes) override;

    void serialWrite(const char *data, size_t length_bytes) override;
};


#endif //XBEE_BACKEND_CPP_RADIOMODULE_H
