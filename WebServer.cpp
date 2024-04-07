//
// Created by William Scheirey on 3/13/24.
//

#include "WebServer.h"
#include "Utility.h"
#include <iostream>

#define DEBUG false

#define connectHelper(method_signature) connect(&server, SIGNAL(method_signature), this, SLOT(method_signature))

WebServer::WebServer(int port, QObject *parent) : QObject(parent), port(port),
                                                  server("Local Server",
                                                         QWebSocketServer::NonSecureMode)
{
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

    connect(this, SIGNAL(broadcast(const QString &)), newSocket, SLOT(sendData(const QString &)));

    clients.append(newSocket);

    connect(newSocket, SIGNAL(stateChanged(WebSocket * , QAbstractSocket::SocketState)), this,
            SLOT(clientStateChanged(WebSocket * , const QAbstractSocket::SocketState &)));

    QObject::connect(socket, &QWebSocket::errorOccurred,
                     [](QAbstractSocket::SocketError error)
                     {
                         qDebug() << "Socket error occurred: " << error;
                     });
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
    auto *thePacket = (TelemPacket *) (data);

    std::string json = JS::serializeStruct(*thePacket);

    dataLogger.dataReady(json.c_str());

    emit broadcast(json.c_str());
}

void WebServer::dataReady(const uint8_t *data, size_t length_bytes, uint8_t rssi)
{
    auto *thePacket = (TelemPacket *) (data);

//    qDebug() << "Loop count: " << Qt::dec << thePacket->loopCount;

    std::string json = JS::serializeStruct(*thePacket);

    dataLogger.dataReady(json.c_str(), rssi);

    for (WebSocket *socket: clients)
    {
        socket->sendData(json.c_str());
    }
//    emit broadcast(json.c_str());
}
