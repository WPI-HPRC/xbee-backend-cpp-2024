//
// Created by William Scheirey on 3/12/24.
//

#ifndef XBEE_BACKEND_CPP_XBEEDEVICE_H
#define XBEE_BACKEND_CPP_XBEEDEVICE_H

#include <QObject>
#include <QSerialPort>
#include <queue>
#include "Utility.h"
#include "XBeeUtility.h"
#include "CircularBuffer.hpp"

#define BUFFER_LENGTH 2048

class XBeeDevice : public QObject
{
Q_OBJECT

public:
    explicit XBeeDevice(QSerialPort *serialPort, QObject *parent = nullptr);

    void sendFrame(uint8_t *frame, size_t size_bytes);

    void sendTransmitRequestCommand(uint64_t address, const uint8_t *data, size_t size_bytes);

    void sendNodeDiscoveryCommand();

    void queueAtCommandLocal(uint16_t command, const uint8_t *commandData, size_t commandDataSize_bytes);

    void
    sendAtCommandLocal(uint8_t frameType, uint16_t command, const uint8_t *commandData, size_t commandDataSize_bytes);

    void sendAtCommandLocal(uint16_t command, const uint8_t *commandData, size_t commandDataSize_bytes);

    void setParameter(uint16_t parameter, const uint8_t *value, size_t valueSize_bytes);

    void setParameter(uint16_t parameter, uint8_t value);

    void queryParameter(uint16_t parameter);

    void applyChanges();

    void write();

    void receive();

public slots:

    void doCycle();


private:
    void serialSend(const char *data, size_t length_bytes);

    bool isProcessingPacket;
    bool shouldSendDiscoveryRequest;

    QSerialPort *m_serialPort;

    uint8_t *transmitRequestFrame;
    uint8_t *atCommandFrame;

    uint8_t currentFrameID;

    std::queue<XBee::BasicFrame *> transmitFrameQueue;
    std::queue<uint16_t> atParamConfirmationsBeingWaitedOn;

    XBee::ReceivePacket::Struct *receivePacketStruct = new XBee::ReceivePacket::Struct;

    char *receiveFrame;
    uint8_t *test;
    char *nodeID;
    TelemPacket *telemPacket;
    SerialCircularBuffer *buffer;

    bool handleFrame(const uint8_t *frame);

    static XBee::BasicFrame createSetATParamValueFrame(uint16_t command, uint8_t value);

    static uint16_t getAtCommand(const uint8_t *frame);

    void parseReceivePacket(const uint8_t *frame, uint8_t length);

    void handleNodeDiscoveryOptionsResponse(const uint8_t *frame, uint8_t length_bytes);

    void handleAtCommandResponse(const uint8_t *frame, uint8_t length_bytes);

    void handleNodeDiscoveryResponse(const uint8_t *frame, uint8_t length_bytes);

    void handleReceivePacket(XBee::ReceivePacket::Struct *frame);


signals:

    void dataReady(const uint8_t *data, size_t length_bytes);

};


#endif //XBEE_BACKEND_CPP_XBEEDEVICE_H
