#ifndef TCPEXPLORER_H
#define TCPEXPLORER_H

#include <QThread>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include "Constants.h"

class TcpExplorer : public QThread {
    Q_OBJECT
public :
    void init(QObject* parent, QString subnet, QString mac, QList<QTcpSocket*>* existingSockets, QHostAddress serverAddress = QHostAddress("")) {
        this->subnet = subnet;
        this->mac = mac;
        this->serverAddress = serverAddress;
        for(int i=0; i<MAX_PIS; i++)
            sockets->append(new QTcpSocket(parent));
        if(!serverAddress.isNull())
            existing->append(serverAddress);
        for(int i=0; i<existingSockets->size();i++)
            existing->append(existingSockets->at(i)->peerAddress());
        ready = true;
        connect(this,SIGNAL(initSocketsSignal()),this,SLOT(initSocket()));
    }

signals:
    void initSocketsSignal();
    void results(QList<QTcpSocket*>);

public slots:
    void run() {
        while(true) {
            while(!ready);

            while(currentIndex<MAX_PIS) {
                if(!pause) {
                    pause = true;
                    emit(initSocketsSignal());
                }
            }
            pause = true;
            emit(disconnectSockets());
            while(pause);
            emit(results(*valid));
            currentIndex = 0;
            wait(TIME_BETWEEN_DISCOVERIES);
        }
    }


private slots:
    void initSocket() {
        QString temp = QString(subnet).replace(subnet.lastIndexOf("0"),1,QString::number(currentIndex));
        if(!existing->contains(QHostAddress(temp))) {
            connect(sockets->at(currentIndex),SIGNAL(connected()),this,SLOT(onSocketConnected()));
            connect(sockets->at(currentIndex),SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onSocketError(QAbstractSocket::SocketError)));
            connect(sockets->at(currentIndex),SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
            sockets->at(currentIndex)->connectToHost(QHostAddress(temp),DEFAULT_PORT);
        } else {
            //qDebug() << currentIndex << " is already connected";
            currentIndex++;
            pause = false;
        }
    }

    void disconnectSockets() {
        for(int i=0; i<sockets->size();i++) {
            disconnect(sockets->at(i),SIGNAL(connected()),this,SLOT(onSocketConnected()));
            disconnect(sockets->at(i),SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onSocketError(QAbstractSocket::SocketError)));
            disconnect(sockets->at(i),SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
        }
        pause = false;
    }

    void onSocketStateChanged(QAbstractSocket::SocketState state) {
        //qDebug() << "Socket state " << currentIndex <<"changed to " << state;
    }

    void onSocketConnected() {
        qDebug() << "TcpExplorer - Connected to " << sockets->at(currentIndex)->peerAddress().toString() << " on port " << ((QTcpSocket*)sender())->peerPort() <<
                    "from " << sockets->at(currentIndex)->localAddress().toString() << " from port " << ((QTcpSocket*)sender())->localPort();
        valid->append(sockets->at(currentIndex));
        sockets->at(currentIndex)->write(QString(SEP_MAC).append(mac).append(SEP_MAC).toLatin1());
        currentIndex++;
        pause = false;
    }

    void onSocketError(QAbstractSocket::SocketError error) {
        //qDebug() << error << "for socket" << currentIndex;
        sockets->at(currentIndex)->abort();
        sockets->at(currentIndex)->close();
        currentIndex++;
        pause = false;
    }



private:
    QList<QTcpSocket*> *valid = new QList<QTcpSocket*>();
    QList<QTcpSocket*> *sockets = new QList<QTcpSocket*>();
    QList<QHostAddress> *existing = new QList<QHostAddress>();
    QString subnet;
    QHostAddress serverAddress;
    int currentIndex = 0;
    bool ready = false;
    QString mac;
    bool pause = false;

};

#endif // TCPEXPLORER_H
