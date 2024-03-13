//
// Created by William Scheirey on 3/13/24.
//

#ifndef XBEE_BACKEND_CPP_WEBSERVER_H
#define XBEE_BACKEND_CPP_WEBSERVER_H

#include <QObject>
#include <QtWebSockets/QtWebSockets>

class WebSocket: public QObject
{
    Q_OBJECT
public:
    QWebSocket *socket;
    explicit WebSocket(QWebSocket *socket): socket(socket)
    {
        connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    }
public slots:
    void stateChanged(QAbstractSocket::SocketState socketState)
    {
        emit stateChanged(this, socketState);
    };

signals:
    void stateChanged(WebSocket *socket, QAbstractSocket::SocketState);
};

class WebServer: public QObject
{
    Q_OBJECT

public:
    WebServer(int port, QObject *parent = nullptr);
    int port;
//    void send(QStrin)

private:
    QWebSocketServer server;
    QList<WebSocket *>clients;

public slots:
    void acceptError(QAbstractSocket::SocketError socketError);

    void alertReceived(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description);

    void alertSent(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description);

    void closed();

    void newConnection();

    void serverError(QWebSocketProtocol::CloseCode closeCode);

    void dataReady(const uint8_t *data, size_t length_bytes);

    void clientStateChanged(WebSocket *socket, const QAbstractSocket::SocketState &state);


};


#endif //XBEE_BACKEND_CPP_WEBSERVER_H
