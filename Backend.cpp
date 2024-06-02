//
// Created by William Scheirey on 3/13/24.
//

#include "Backend.h"
#include "DataSimulator.h"
#include <iostream>

#define DEBUG_SERIAL false

#define SIMULATE_DATA false

Backend::Backend(QObject *parent) : QObject(parent)
{
#if SIMULATE_DATA
    webServer = new WebServer(8001);

    dataSimulator = new DataSimulator(
            "/Users/will/Documents/GitHub/HPRC/telemetry-server/logs/2024-02-24_18.02.19_telemetry.csv", 25, webServer);
    return;
#else

    webServer = new WebServer(8001);

    QString timeString = QDateTime::currentDateTime().toString(Constants::LogTimeFormat);

    dataLogger = new DataLogger("", false);

    DataLogger::enclosingDirectory = dataLogger->logDir.absolutePath();

    auto *rocketModule = new RadioModule(115200, new DataLogger("Rocket"));
    rocketModule->name = "Rocket";

    auto *payloadModule = new RadioModule(115200, new DataLogger("Payload"));
    payloadModule->name = "Payload";

    auto *groundModule = new ServingRadioModule(921600, new DataLogger("Ground_Station"), webServer);
    groundModule->name = "Ground_Station";

    radioModules.append(rocketModule);
    radioModules.append(payloadModule);
    radioModules.append(groundModule);

    timer = new QTimer();
    timer->setInterval(5);
    connect(timer, &QTimer::timeout, [this]()
            {
                for (auto radioModule: this->radioModules)
                {
                    radioModule->doCycle();
                }
            }
    );
    timer->start();
#endif
}
