#ifndef TCP_HOST_H
#define TCP_HOST_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QNetworkInterface>
#include <QElapsedTimer>
#include "GlobalVars.h"
#include "TaskWrapper.h"
#include "LedsController.h"


class TcpHost : public QObject {
    Q_OBJECT
    public:
        TcpHost();
    signals:
        void clientConnected(QString,QString);
        void clientDisconnected(QString);
        void serverOnline();
        void localIp(int);
        void connectTo(QHostAddress);
        void connectedPis(QList<QPair<QTcpSocket*,QString>>*, int sender);
    public slots:
        void broadcast(QString data);
        void startServer(QHostAddress);
        void requestConnectedPis(int sender);
        void setCommandShift(bool shft);
        void socketDisconnected(QString);
    private slots:
        void onClientConnected();
        void onClientDisconnected();
        void onClientDisconnected(QTcpSocket *socket);

        void write(QTcpSocket *socket, QString data);
        void onClientConnectionError(QAbstractSocket::SocketError error);
        void read();
        bool isComplete(QString str);
        int findIndex(QTcpSocket *socket);
        int isDeviceConnected(int dev);
        int contains(QList<QPair<int, int> > candidates, int i);
private:
        QTcpServer *server;
        QHostAddress *address;
        QList<QPair<QTcpSocket*,QString>> *devices;
        QString buffer;
        int port;
        bool cmdShft = false;
        int localAd;
};

#endif // TCP_HOST_H
