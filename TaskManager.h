#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QDebug>
#include "TaskWrapper.h"

class TaskManager : public QObject {
        Q_OBJECT
    public:
        TaskManager();
    signals:
        void giveTask(QString);
        void cancelGive(long);
    public slots:
        void receiveTask(QString);
        void receiveTask(TaskWrapper*);
        void localIp(int);
    private slots:
        void doTask(QString id, int value, long taskId);
        void cancelDo(long);
        void cancelGive(TaskWrapper*);
    private:
        QList<TaskWrapper*>* tasks;
        int ip;
};

#endif // TASKMANAGER_H
