#include "TaskManager.h"

bool DEBUG_MODE= false;

TaskManager::TaskManager() {
    syncedTasks = new QList<TaskWrapper*>();
    servo = new ServoThread;
    servo->init();
    wiringPiSetup();

    pinMode(PWM_PIN, PWM_OUTPUT);
    pwmWrite(PWM_PIN, PWM_INITIAL_VALUE);
}

void TaskManager::receiveTask(QString s, int sender) {
    receiveTask(new TaskWrapper(s),sender);
}

void TaskManager::receiveTask(TaskWrapper* t, int sender) {
    if(t->getId()=="servo_sync"&&sender==SENDER_BT) {
        syncedTasks->append(t);
        if(syncedTasks->size()==2)
            doSyncedTasks();
        return;
    }



    if(t->getTarget()==ip) {
        if(t->getType()==TYPE_INT)
            doTask(t->getId(), t->getValue());
        else if(t->getType()==TYPE_STR)
            doTask(t->getId(), t->getStrValue());
    } else if(t->getTarget()==TARGET_ALL) {
        if(sender==SENDER_BT)   // évite répercussions
            emit(giveTask(t->toString()));

        if(t->getType()==TYPE_INT)
            doTask(t->getId(), t->getValue());
        else if(t->getType()==TYPE_STR)
            doTask(t->getId(), t->getStrValue());
    } else if(sender==SENDER_BT)
        emit(giveTask(t->toString()));

}

void TaskManager::localIp(int i) {
    ip = i;
}


/** DO */
void TaskManager::doTask(QString id, int value) {
    //qDebug() << "TaskManager - Wrote " << value << " on " << id;

    if(id=="cmd_shft")
        emit(enableShifting(value));
    else if(id=="dbg")
        DEBUG_MODE = value==1;
    else if(id=="servo_a") {
        pwmWrite(PWM_PIN, value);
        if(DEBUG_MODE)
            qDebug() << "Servo - " << QString::number(value);
    }
    else if(id=="servo_s") {
        if(DEBUG_MODE)
            qDebug() << "ServoSpeed = " << value;
        servo->setSpeed(value);
    }


}

void TaskManager::doTask(QString id, QString value) {
    //qDebug() << "TaskManager - Wrote " << value;

    if(id=="leds")
        (new QProcess())->startDetached(QString("sudo /home/pi/").append(value));
    else if(id=="servo") {
        if(!servo->isRunning())
            servo->start();
        servo->setCmd(value);
    } else if(id=="servo_sync") {
        if(!servo->isRunning())
            servo->start();
        servo->setSyncedCmd(value);
    }
}


/** SYNCED */
void TaskManager::doSyncedTasks() {
    emit(requestConnectedPis(SENDER_TASKM));
}

// TODO standby
void TaskManager::connectedPis(QList<QPair<QTcpSocket *, QString> > * list, int sender) {
    if(sender == SENDER_TASKM) {
        if(isDeviceConnected(list, syncedTasks->at(0)->getTarget())!=-1&&isDeviceConnected(list, syncedTasks->at(1)->getTarget())!=-1) {
            receiveTask(syncedTasks->at(0), SENDER_TASKM);
            receiveTask(syncedTasks->at(1), SENDER_TASKM);
            if(DEBUG_MODE)
                qDebug() << "Devices were both online";
        } else
            if(DEBUG_MODE)
                qDebug() << "A device was not online";
        syncedTasks->clear();
    }
}

void TaskManager::piDisconnected(QString s) {
    servo->onDeviceDisconnected(s.toInt());
}

int TaskManager::isDeviceConnected(QList<QPair<QTcpSocket *, QString> > * list, int dev) {
    qDebug() << "Looking for " << QString::number(dev);
    for(int j=0; j<list->size(); j++)
        if(list->at(j).second.right(1).toInt()==ip||list->at(j).first->peerAddress().toString().right(1).toInt()==dev) {
            qDebug() << "Returning " << QString::number(j);
            return j;
        }
    qDebug() << "Returning " << QString::number(-1);
    return -1;
}
