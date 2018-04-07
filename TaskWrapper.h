#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QDebug>
#include "GlobalVars.h"

#define TYPE_INT 0
#define TYPE_STR 1

class TaskWrapper : public QObject {
    Q_OBJECT
public:
    TaskWrapper(QString id, int value, int target) {
        this->id = id;
        this->value = value;
        this->target = target;
        this->type = TYPE_INT;
    }

    TaskWrapper(QString id, QString value, int target) {
        this->id = id;
        this->valueStr = value;
        this->target = target;
        this->type = TYPE_STR;
    }

    TaskWrapper(QString s) {
        type = s.section(SEP_TASK_ITEM,1,1).replace(SEP_TASK_ITEM,"").replace(SEP_TASK,"").toInt();
        id = s.section(SEP_TASK_ITEM,2,2).replace(SEP_TASK_ITEM,"");
        if(type==TYPE_INT)
            value = s.section(SEP_TASK_ITEM,3,3).replace(SEP_TASK_ITEM,"").toInt();
        else //if(type==TYPE_STR)
            valueStr = s.section(SEP_TASK_ITEM,3,3).replace(SEP_TASK_ITEM,"");
        target = s.section(SEP_TASK_ITEM,4,4).replace(SEP_TASK_ITEM,"").replace(SEP_ENDTASK,"").toInt();
    }

    bool operator==(TaskWrapper &other) {
        return toString()==other.toString();
    }

    static int getTarget(QString task) {
        return (new TaskWrapper(task))->getTarget();
    }

    static bool isValidTasks(QString tasks) {
        return tasks.count(SEP_TASK)>=1&&tasks.count(SEP_ENDTASK)>=1;
    }

    static bool isValidTask(QString task) {
        return task.count(SEP_TASK)==1&&task.count(SEP_ENDTASK)==1;
    }

public slots:
    QString toString() {
        if(type==TYPE_INT)
            return QString(SEP_TASK).append(SEP_TASK_ITEM).append(QString::number(type)).append(SEP_TASK_ITEM).append(id).append(SEP_TASK_ITEM)
                    .append(QString::number(value)).append(SEP_TASK_ITEM).append(QString::number(target)).append(SEP_ENDTASK);
        else //if(type==TYPE_STR)
            return QString(SEP_TASK).append(SEP_TASK_ITEM).append(QString::number(type)).append(SEP_TASK_ITEM).append(id).append(SEP_TASK_ITEM)
                    .append(valueStr).append(SEP_TASK_ITEM).append(QString::number(target)).append(SEP_ENDTASK);
    }

    QString getId() {
        return id;
    }

    QString getStrValue() {
        return valueStr;
    }

    int getValue() {
        return value;
    }

    int getTarget() {
        return target;
    }

    int getType() {
        return type;
    }


private:
    QString id;
    int value;
    QString valueStr;
    int target;
    int type;
};


#endif // TASK_H
