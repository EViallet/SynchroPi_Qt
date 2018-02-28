#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "Constants.h"

class TaskWrapper : public QObject {
    Q_OBJECT
public:
    TaskWrapper(long taskId, QString id, int value, int target, long millis = 0, bool singleShot = true) {
        this->taskId = taskId;
        this->id = id;
        this->value = value;
        this->target = target;
        this->millis = millis;
        this->singleShot = singleShot||millis==0;
        if(millis>0) {
            connect(&timer,SIGNAL(timeout()),this,SLOT(trigger()));
            timer.setSingleShot(singleShot);
            timer.setInterval(millis);
        }
    }

    TaskWrapper(QString s) {
        id = s.section(SEP_TASK_ITEM,1,1).replace(SEP_TASK_ITEM,"").replace(SEP_TASK,"");
        value = s.section(SEP_TASK_ITEM,2,2).replace(SEP_TASK_ITEM,"").toInt();
        target = s.section(SEP_TASK_ITEM,3,3).replace(SEP_TASK_ITEM,"").toInt();
        millis = s.section(SEP_TASK_ITEM,4,4).replace(SEP_TASK_ITEM,"").toLong();
        if(s.section(SEP_TASK_ITEM,5,5).replace(SEP_TASK_ITEM,"").replace(SEP_ENDTASK,"")=="t")
            singleShot = true;
        else
            singleShot = false;

        this->singleShot = singleShot||millis==0;
        if(millis>0) {
            connect(&timer,SIGNAL(timeout()),this,SLOT(trigger()));
            timer.setSingleShot(singleShot);
            timer.setInterval(millis);
        }
    }

    bool operator==(TaskWrapper &other) {
        return toString()==other.toString();
    }

    static int contains(QList<TaskWrapper*>* list, long id) {
        for(int i=0; i<list->size(); i++)
            if(list->at(i)->getTaskId()==id)
                return i;
        return -1;
    }

    static bool isTarget(QString task, int i) {
        return i==-1||(new TaskWrapper(task))->getTarget()==i;
    }

    static bool isValidTasks(QString tasks) {
        qDebug() << "isValidTasks?" << tasks.count(SEP_TASK);
        return tasks.count(SEP_TASK)>=1&&tasks.count(SEP_ENDTASK)>=1;
    }

    static bool isValidTask(QString task) {
        qDebug() << "isValidTask?" << task.count(SEP_TASK);
        return task.count(SEP_TASK)==1&&task.count(SEP_ENDTASK)==1;
    }

public slots:
    QString toString() {
        QString ss = singleShot?"t":"f";
        return SEP_TASK+QString::number(taskId)+SEP_TASK_ITEM+id+SEP_TASK_ITEM+QString::number(value)+SEP_TASK_ITEM+QString::number(target)+SEP_TASK_ITEM+QString::number(millis)+SEP_TASK_ITEM+ss+SEP_ENDTASK;
    }

    bool isScheduled() {
        return millis>0;
    }

    bool isRepetitive() {
        return singleShot;
    }

    int getTarget() {
        return target;
    }

    void registered() {
        if(isScheduled())
            timer.start();
        else
            trigger();
    }

    void unregister() {
        timer.stop();
    }

    long getTaskId() {
        return taskId;
    }

signals:
    void fireTask(QString,int,long);
private slots:
    void trigger() {
        emit(fireTask(id,value,taskId));
    }
private:
    QString id;
    int value;
    long taskId;
    long millis;
    bool singleShot;
    QTimer timer;
    int target;
};


#endif // TASK_H
