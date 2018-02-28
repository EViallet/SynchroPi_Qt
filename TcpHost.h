#ifndef TCP_HOST_H
#define TCP_HOST_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QNetworkInterface>
#include <QTimer>
#include "Constants.h"
#include "TaskWrapper.h"


class TcpHost : public QObject {
    Q_OBJECT
    public:
        TcpHost();
    signals:
        void clientConnected(QString,QString);
        void clientDisconnected(QString);
        void serverOnline();
        void localIp(int);
        void connectedPis(QList<QPair<QTcpSocket*,QString>>*);
    public slots:
        void broadcast(QString data);
        void broadcast(long taskId);
        void startServer(QHostAddress);
        void requestConnectedPis();
    private slots:
        void onClientConnected();
        void onClientDisconnected();
        void onClientDisconnected(QTcpSocket *socket);
        void onClientStateChanged(QAbstractSocket::SocketState state);

        void write(QTcpSocket *socket, QString data);
        void onClientConnectionError(QAbstractSocket::SocketError error);
        void read();
        bool isComplete(QString str);
        int findIndex(QTcpSocket *socket);
private:
        QTcpServer *server;
        QHostAddress *address;
        QList<QPair<QTcpSocket*,QString>> *devices;
        QString buffer;
        int port;
};

#endif // TCP_HOST_H
