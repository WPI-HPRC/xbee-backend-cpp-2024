//
// Created by William Scheirey on 3/13/24.
//

#ifndef XBEE_BACKEND_CPP_BACKEND_H
#define XBEE_BACKEND_CPP_BACKEND_H

#include <Qt>
#include <QObject>

#include "RadioModule.h"
#include "WebServer.h"
#include "DataSimulator.h"
#include "DataLogger.h"

class Backend : public QObject
{
public:
    explicit Backend(QObject *parent = nullptr);

private:
    WebServer *webServer;
    DataSimulator *dataSimulator;
    DataLogger *dataLogger;

    RadioModule *radioModule;
    QTimer *timer;

};


#endif //XBEE_BACKEND_CPP_BACKEND_H
