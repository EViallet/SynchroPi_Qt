#include <QCoreApplication>
#include <QtGlobal>
#include <QThread>
#include <stdlib.h>
#include "BluetoothHandler.h"
#include "TcpHost.h"
#include "TcpClient.h"
#include "TaskManager.h"
#include "GlobalVars.h"

// TODO change BT/TCP paquets

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    //qsrand(time(0));
    //QString ip = QString("sudo ifconfig eth0 %1.%2.%3.%4 up").arg(qrand()%192).arg(qrand()%255).arg(qrand()%255).arg(qrand()%254);
    //system(ip.toLatin1().data());

    BluetoothHandler *bt = new BluetoothHandler;
    TcpHost *host = new TcpHost;
    TcpClient *client = new TcpClient;
    client->init();
    TaskManager *taskM = new TaskManager;


    QObject* linker = new QObject;
    linker->connect(client,SIGNAL(startServer(QHostAddress)),host,SLOT(startServer(QHostAddress)));
    linker->connect(bt,SIGNAL(btAddress(QString)),client,SLOT(getMac(QString)));
    linker->connect(bt,SIGNAL(requestConnectedPis(int)),host,SLOT(requestConnectedPis(int)));
    linker->connect(host,SIGNAL(connectedPis(QList<QPair<QTcpSocket*,QString>>*,int)),bt,SLOT(connectedPis(QList<QPair<QTcpSocket*,QString>>*,int)));
    linker->connect(host,SIGNAL(serverOnline()),client,SLOT(onServerOnline()));
    linker->connect(host,SIGNAL(connectTo(QHostAddress)),client,SLOT(connectTo(QHostAddress)));
    linker->connect(client,SIGNAL(socketDisconnected(QString)),host,SLOT(socketDisconnected(QString)));
    linker->connect(host,SIGNAL(localIp(int)),bt,SLOT(localIp(int)));
    linker->connect(host,SIGNAL(localIp(int)),taskM,SLOT(localIp(int)));
    linker->connect(host,SIGNAL(clientConnected(QString,QString)),bt,SLOT(tcpConnectionAlert(QString,QString)));
    linker->connect(host,SIGNAL(clientDisconnected(QString)),bt,SLOT(tcpDisconnectionAlert(QString)));
    linker->connect(host,SIGNAL(clientDisconnected(QString)),taskM,SLOT(piDisconnected(QString)));
    linker->connect(client,SIGNAL(receivedTask(QString,int)),taskM,SLOT(receiveTask(QString,int)));
    linker->connect(taskM,SIGNAL(giveTask(QString)),host,SLOT(broadcast(QString)));
    linker->connect(taskM,SIGNAL(requestConnectedPis(int)),host,SLOT(requestConnectedPis(int)));
    linker->connect(host,SIGNAL(connectedPis(QList<QPair<QTcpSocket*,QString>>*,int)),taskM,SLOT(connectedPis(QList<QPair<QTcpSocket*,QString>>*,int)));
    linker->connect(bt,SIGNAL(onTaskReceived(TaskWrapper*,int)),taskM,SLOT(receiveTask(TaskWrapper*,int)));

    linker->connect(taskM,SIGNAL(enableShifting(bool)),host,SLOT(setCommandShift(bool)));

    bt->startServer();


    return a.exec();
}
