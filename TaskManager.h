#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QTcpSocket>
#include <QHostAddress>
#include <wiringPi.h>
#include "TaskWrapper.h"
#include "ServoThread.h"


class TaskManager : public QObject {
        Q_OBJECT
    public:
        TaskManager();
    signals:
        void giveTask(QString);
        void enableShifting(bool);
        void requestConnectedPis(int);
    public slots:
        void receiveTask(QString, int sender = SENDER_TCP);
        void receiveTask(TaskWrapper*, int sender);
        void localIp(int);
        void connectedPis(QList<QPair<QTcpSocket*,QString>>*,int);
        void piDisconnected(QString);
    private slots:
        void doTask(QString id, int value);
        void doTask(QString id, QString value);
        void doSyncedTasks();
        int isDeviceConnected(QList<QPair<QTcpSocket *, QString> > *list, int dev);
    private:
        QList<TaskWrapper*>* syncedTasks;
        ServoThread *servo;
        int ip;
};

#endif // TASKMANAGER_H
