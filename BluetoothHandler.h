#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <QtBluetooth>
#include <QDebug>
#include <QList>
#include <QIODevice>
#include <QTcpSocket>
#include <QHostAddress>
#include "TaskWrapper.h"
#include "Constants.h"


class BluetoothHandler : public QObject {
    Q_OBJECT
    public:
        BluetoothHandler();
        ~BluetoothHandler();
    signals:
        void onTaskReceived(TaskWrapper*);
        void btAddress(QString);
        void requestConnectedPis();
    public slots:
        void tcpConnectionAlert(QString mac,QString ip);
        void tcpDisconnectionAlert(QString);
        void startServer();
        void connectedPis(QList<QPair<QTcpSocket*,QString>>*);
        void localIp(int);
    private slots:
        void onClientConnected();
        void onSocketStateChanged(QBluetoothSocket::SocketState state);
        void disconnected();

        bool isComplete(QString str);
        void write(QString data);
        void read();
        void convertTask(long taskId, QString id, int cmd, QList<QPair<int, int> > *targets);
        void convertTask(long taskid, QString id, bool cmd, QList<QPair<int, int> > *targets);

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
