#include <QCoreApplication>
#include <QtGlobal>
#include <QThread>
#include <stdlib.h>
#include "BluetoothHandler.h"
#include "BluetoothCtl.h"
#include "TcpHost.h"
#include "TcpClient.h"
#include "TaskManager.h"
#include "Constants.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    qsrand(time(0));
    QString ip = QString("sudo ifconfig eth0 %1.%2.%3.%4 up").arg(qrand()%192).arg(qrand()%255).arg(qrand()%255).arg(qrand()%254);
    system(ip.toLatin1().data());

    //BluetoothCtl *btCtl = new BluetoothCtl;
    BluetoothHandler *bt = new BluetoothHandler;
    TcpHost *host = new TcpHost;
    TcpClient *client = new TcpClient;
    TaskManager *taskM = new TaskManager;


    QObject* linker = new QObject;
    linker->connect(client,SIGNAL(startServer(QHostAddress)),host,SLOT(startServer(QHostAddress)));
    linker->connect(bt,SIGNAL(btAddress(QString)),client,SLOT(getMac(QString)));
    linker->connect(bt,SIGNAL(requestConnectedPis()),host,SLOT(requestConnectedPis()));
    linker->connect(host,SIGNAL(connectedPis(QList<QPair<QTcpSocket*,QString>>*)),bt,SLOT(connectedPis(QList<QPair<QTcpSocket*,QString>>*)));
    linker->connect(host,SIGNAL(serverOnline()),client,SLOT(discover()));
    linker->connect(host,SIGNAL(localIp(int)),bt,SLOT(localIp(int)));
    linker->connect(host,SIGNAL(localIp(int)),taskM,SLOT(localIp(int)));
    linker->connect(host,SIGNAL(clientConnected(QString,QString)),bt,SLOT(tcpConnectionAlert(QString,QString)));
    linker->connect(host,SIGNAL(clientDisconnected(QString)),bt,SLOT(tcpDisconnectionAlert(QString)));
    linker->connect(client,SIGNAL(receivedTask(QString)),taskM,SLOT(receiveTask(QString)));
    linker->connect(client,SIGNAL(receivedCancel(long)),taskM,SLOT(cancelDo(long)));
    linker->connect(taskM,SIGNAL(giveTask(QString)),host,SLOT(broadcast(QString)));
    linker->connect(taskM,SIGNAL(cancelGive(long)),host,SLOT(broadcast(long)));
    linker->connect(bt,SIGNAL(onTaskReceived(TaskWrapper*)),taskM,SLOT(receiveTask(TaskWrapper*)));

    //btCtl->start();
    bt->startServer();


    return a.exec();
}
