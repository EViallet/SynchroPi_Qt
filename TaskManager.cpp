#include "TaskManager.h"

TaskManager::TaskManager() {
    tasks = new QList<TaskWrapper*>();
}

void TaskManager::receiveTask(QString s) {
    receiveTask(new TaskWrapper(s));
}

void TaskManager::receiveTask(TaskWrapper* t) {
    if(t->getTarget()==ip) {
        t->connect(t,SIGNAL(fireTask(QString,int,long)),this,SLOT(doTask(QString,int,long)));
        tasks->append(t);
        t->registered();
    } else
        emit(giveTask(t->toString()));
}

void TaskManager::localIp(int i) {
    ip = i;
}

/** GIVE */
void TaskManager::cancelGive(TaskWrapper *t) {
    emit(cancelGive(t->getTaskId()));
}

/** DO */

void TaskManager::doTask(QString id, int value, long taskId) {
    qDebug() << "TaskManager - Wrote " << value << " on " << id;

    int index = TaskWrapper::contains(tasks,taskId);
    if(index!=-1) {
        if(!tasks->at(index)->isRepetitive())
            tasks->removeAt(index);
    }

}

void TaskManager::cancelDo(long taskId) {
    int index = TaskWrapper::contains(tasks,taskId);
    if(index!=-1) {
        tasks->at(index)->unregister();
        tasks->removeAt(index);
    }
}
