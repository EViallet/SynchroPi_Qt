#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QThread>
#include <QFile>
#include <QTimer>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <limits>
#include "TaskWrapper.h"
#include "GlobalVars.h"

class TcpClient : public QThread {
    Q_OBJECT
    public:
        void run() override;
    signals:
        void pingSocketSignal(int);
        void receivedTask(QString,int);
        void receivedCancel(long);
        void startServer(QHostAddress);
        void initSocketsSignal();
        void socketDisconnected(QString);
        void startUnpauseThreadTimer();
    public slots:
        void firstDiscovery();
        void getMac(QString mac);
        void init();
        void connectTo(QHostAddress a);
		void onServerOnline();
    private slots:
        void initSocket();
        void onSocketConnected();
        void onSocketDisconnected(QTcpSocket *socket);
        void onSocketError(QAbstractSocket::SocketError error);
        void onSocketStateChanged(QAbstractSocket::SocketState state);

        void read();
        void availableAddress();
        bool contains();
        void onTimeout();
        void pingSocket(int pos);
        void unpauseThread();
private:
        QList<QTcpSocket*> *sockets = new QList<QTcpSocket*>();
        QList<QTimer*> *timers = new QList<QTimer*>();
        QTimer *threadPauseTimer;
        QString buffer = "";
        QString mac;
        int server = -1;
        bool serverListening = false;
        bool pauseThread = false;
};

#endif // TCPCLIENT_H
