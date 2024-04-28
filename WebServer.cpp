//
// Created by William Scheirey on 3/13/24.
//

#include "WebServer.h"
#include "Utility.h"
#include <iostream>
#include <regex>

#define DEBUG false

#define connectHelper(method_signature) connect(&server, SIGNAL(method_signature), this, SLOT(method_signature))

WebServer::WebServer(int port, QObject *parent) : QObject(parent), port(port),
                                                  server("Local Server",
                                                         QWebSocketServer::NonSecureMode)
{

    serverSocket = new HPRCWebSocket();

    serverSocket->connectToServer();

    server.listen(QHostAddress::LocalHost, port);

    std::cout << "Started local server on " << server.serverAddress().toString().toStdString() << ":"
              << server.serverPort() << std::endl;

#if DEBUG
    connectHelper(acceptError(QAbstractSocket::SocketError));
    connectHelper(alertReceived(QSsl::AlertLevel, QSsl::AlertType, const QString));
    connectHelper(alertSent(QSsl::AlertLevel, QSsl::AlertType, const QString));
    connectHelper(closed());
    connectHelper(serverError(QWebSocketProtocol::CloseCode));
#endif

    connectHelper(newConnection());
}

void WebServer::acceptError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Accepted error: " << socketError;
}

void WebServer::newConnection()
{
    QWebSocket *socket = server.nextPendingConnection();

    if (socket == nullptr)
    {
        return;
    }
#if DEBUG
    qDebug() << "Socket connected: " << socket->peerAddress().toString();
#endif

    auto *newSocket = new WebSocket(socket);

//    connect(this, SIGNAL(broadcast(const QString &)), newSocket, SLOT(sendData(const QString &)));

    clients.append(newSocket);

    connect(newSocket, SIGNAL(stateChanged(WebSocket * , QAbstractSocket::SocketState)), this,
            SLOT(clientStateChanged(WebSocket * , const QAbstractSocket::SocketState &)));

    /*
    QObject::connect(socket, &QWebSocket::errorOccurred,
                     [](QAbstractSocket::SocketError error)
                     {
                         qDebug() << "Socket error occurred: " << error;
                     });
*/
}

void WebServer::alertReceived(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description)
{
    qDebug() << "Alert received: " << description;
}

void
WebServer::alertSent(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description)
{
    qDebug() << "Alert sent: " << description;
}

void WebServer::closed()
{
    qDebug() << "Closed";
}

void WebServer::serverError(QWebSocketProtocol::CloseCode closeCode)
{
    qDebug() << "Server error: " << closeCode;
}

void WebServer::clientStateChanged(WebSocket *socket, const QAbstractSocket::SocketState &state)
{
    if (state == QAbstractSocket::UnconnectedState)
    {
        qDebug() << "Socket disconnected: " << socket->socket->peerAddress().toString();
        clients.remove(clients.indexOf(socket));
    }
}

void WebServer::dataReady(const uint8_t *data, size_t length_bytes)
{
    std::string json;
    DataLogger::PacketType packetType;
    if (data[0] == 0x01) // Rocket packet
    {
        json = JS::serializeStruct(*(RocketTelemPacket *) (&data[1]));
        packetType = DataLogger::Rocket;
    }
    else if (data[0] == 0x02) // Payload packet
    {
        json = JS::serializeStruct(*(PayloadTelemPacket *) (&data[1]));
        packetType = DataLogger::Payload;
    }
    else
    {
//        qDebug() << "Unrecognized: " << Qt::hex << (int) (data[0] & 0xFF);
        return;
    }


    json = std::regex_replace(json, std::regex("nan"), "0");
    json = std::regex_replace(json, std::regex("inf"), "0");

    dataLogger.dataReady(json.c_str(), packetType);

    broadcast(json.c_str());
}

void WebServer::dataReady(const uint8_t *data, size_t length_bytes, uint8_t rssi)
{
    DataLogger::PacketType packetType;
    std::string json;
    if (data[0] == 0x01) // Rocket packet
    {
        json = JS::serializeStruct(*(RocketTelemPacket *) (&data[1]));
        packetType = DataLogger::Rocket;
    }
    else if (data[0] == 0x02) // Payload packet
    {
        json = JS::serializeStruct(*(PayloadTelemPacket *) (&data[1]));
        packetType = DataLogger::Payload;
    }
    else
    {
        qDebug() << "Unrecognized: " << Qt::hex << (int) (data[0] & 0xFF);
        return;
    }

    json = std::regex_replace(json, std::regex("nan"), "0");
    json = std::regex_replace(json, std::regex("inf"), "0");
    dataLogger.dataReady(json.c_str(), packetType, rssi);

    broadcast(json.c_str());
}

void WebServer::broadcast(const QString &str)
{
    serverSocket->m_socket->sendTextMessage(str);
    for (WebSocket *socket: clients)
    {
        socket->sendData(str);
    }
}
