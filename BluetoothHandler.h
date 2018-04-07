#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <QtBluetooth>
#include <QDebug>
#include <QList>
#include <QIODevice>
#include <QTcpSocket>
#include <QHostAddress>
#include "TaskWrapper.h"
#include "GlobalVars.h"
#include "LedsController.h"


class BluetoothHandler : public QObject {
    Q_OBJECT
    public:
        BluetoothHandler();
        ~BluetoothHandler();
    signals:
        void onTaskReceived(TaskWrapper*,int);
        void btAddress(QString);
        void requestConnectedPis(int);
    public slots:
        void tcpConnectionAlert(QString mac,QString ip);
        void tcpDisconnectionAlert(QString);
        void startServer();
        void connectedPis(QList<QPair<QTcpSocket*,QString>>*, int);
        void localIp(int);
    private slots:
        void onClientConnected();
        void onSocketStateChanged(QBluetoothSocket::SocketState state);
        void disconnected();

        bool isComplete(QString str);
        void write(QString data);
        void read();
        void convertTask(QString id, int cmd, QList<int> *targets);
        void convertTask(QString id, bool cmd, QList<int> *targets);
        void convertTask(QString id, QString cmd, QList<int> *targets);

        void terminateProgram();
        void shutdown();

        bool isCmdComplete(QString str);
        void tcpHeader();
private:
        bool firstStart = false;
        QBluetoothLocalDevice raspberry;
        QBluetoothAddress android;
        QBluetoothServer *server;
        QBluetoothSocket *socket = nullptr;
        QString buffer;
        bool connecting;
        int serverIp;
};

#endif // BLUETOOTH_HANDLER_H
