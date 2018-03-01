#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QFile>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <limits>
#include "TcpExplorer.h"
#include "TaskWrapper.h"
#include "Constants.h"

class TcpClient : public QObject {
    Q_OBJECT
    public:
        TcpClient();
    signals:
        void receivedTask(QString);
        void receivedCancel(long);
        void startServer(QHostAddress);
    public slots:
        void firstDiscovery();
        void discover();
        void getMac(QString mac);
    private slots:
        void onSocketDisconnected(QTcpSocket *socket);
        void onSocketStateChanged(QAbstractSocket::SocketState state);

        void read();
        void newSockets(QList<QTcpSocket *> sockets);
        void findAvailableAddress(QList<QTcpSocket *> addresses);

        void ping();
    private:
        QList<QTcpSocket*> *sockets;
        QHostAddress serverAddress;
        QString buffer = "";
        int server;
        TcpExplorer *explorer;
        QTimer pinger;
        QString mac;
};

#endif // TCPCLIENT_H
