//
// Created by William Scheirey on 3/13/24.
//

#include "Backend.h"
#include "DataSimulator.h"
#include <iostream>

#define DEBUG_SERIAL false

Backend::Backend(QObject *parent) : QObject(parent)
{
//    webServer = new WebServer(8001);

//    dataSimulator = new DataSimulator("/Users/will/Documents/GitHub/HPRC/telemetry-server/logs/2024-02-24_18.02.19_telemetry.csv", 25, webServer);
//    return;

    radioModule = new RadioModule();

    timer = new QTimer();
    timer->setInterval(10);
    connect(timer, &QTimer::timeout, [this]()
            {
                this->radioModule->doCycle();
            }
    );
    timer->start();
}
