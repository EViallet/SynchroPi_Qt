#include "BluetoothHandler.h"

BluetoothHandler::BluetoothHandler() {
    raspberry.powerOn();
}

/** BLUETOOTH CONNECTION MANAGING */
void BluetoothHandler::startServer() {
    if(!firstStart) {
        firstStart = true;
        emit(btAddress(raspberry.address().toString()));
    }
    connecting = true;
    raspberry.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

    server = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol);
    connect(server,SIGNAL(newConnection()),this,SLOT(onClientConnected()));
    server->listen(QBluetoothUuid(QString(UUID)));
    qDebug() << "BT - Server online";
}

void BluetoothHandler::connectedPis(QList<QPair<QTcpSocket*, QString>> *devices) {
    tcpHeader();
    for(int i=0; i<devices->size();i++)
        tcpConnectionAlert(devices->at(i).second,devices->at(i).first->peerAddress().toString().right(1));
}

void BluetoothHandler::localIp(int serverIp) {
    this->serverIp = serverIp;
}

void BluetoothHandler::onClientConnected() {
    socket = server->nextPendingConnection();

    qDebug() << "BT - Connected";
    emit(requestConnectedPis());
    connect(socket,SIGNAL(readyRead()),this,SLOT(read()));
    connect(socket,SIGNAL(stateChanged(QBluetoothSocket::SocketState)),this,SLOT(onSocketStateChanged(QBluetoothSocket::SocketState)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(disconnected()));
    server->close();
    raspberry.setHostMode(QBluetoothLocalDevice::HostConnectable);
}

void BluetoothHandler::onSocketStateChanged(QBluetoothSocket::SocketState state) {
    /*if(state==QBluetoothSocket::UnconnectedState||state==QBluetoothSocket::ClosingState)
        disconnected();*/
}

void BluetoothHandler::disconnected() {
    qDebug() << "BT - Disconnected";
    //if(socket->isOpen())
    //    socket->close();
    startServer();
}


/** DATA HANDLING */
bool BluetoothHandler::isComplete(QString str) {
    return str.count(SEP_PACKET)>0&&str.count(SEP_TASKID)>=2&&(str.count(SEP_DBG)>=2||(str.count(SEP_ID)>=2&&(str.count(SEP_INT)>=2||str.count(SEP_STR)>=2||str.count(SEP_BOOL)>=2)));
}

bool BluetoothHandler::isCmdComplete(QString str) {
    return str.count(SEP_TASKID)==2&&(str.count(SEP_DBG)==2||str.count(SEP_ID)==2&&(str.count(SEP_INT)==2||str.count(SEP_STR)==2||str.count(SEP_BOOL)==2));
}


void BluetoothHandler::write(QString data) {
    if(socket!=nullptr&&socket->isOpen()) {
        qDebug() << "BT - Writing " << data;
        socket->write(QString(SEP_PACKET).append(data).toLatin1());
    }
}


void BluetoothHandler::read() {
    QString str (socket->readAll());

    str.replace(PING_CHAR,"");

    bool valid = false;
    QString read = "";


    if(!isComplete(str)) {
        buffer.append(str);
        if(!isComplete(buffer))
            return;
        else {
            read = buffer;
            valid = true;
            buffer = "";
        }
    } else {
        read = str;
        valid = true;
    }

    if(valid) {
        QStringList cmds = read.split(SEP_PACKET);
        for(int i=1; i<cmds.length(); i++) {
            QString strCmd = cmds.at(i);
            qDebug() << "BT - Received : " << strCmd;
            if(!isCmdComplete(strCmd)) {
                buffer.append(strCmd);
                break;
            }
            strCmd = strCmd.replace(SEP_PACKET,"");
            if (strCmd.contains(SEP_DBG)) {
                QString cmd = strCmd.replace(SEP_DBG,"");
                if(cmd=="Shutdown")
                    shutdown();
            } else {
                if (strCmd.contains(SEP_INT)) {
                    QString id = strCmd.section(SEP_ID, 1, 1).replace(SEP_ID,"");
                    long taskid = strCmd.section(SEP_TASKID,1,1).replace(SEP_TASKID,"").toLong();
                    int cmd = strCmd.section(SEP_INT, 1, 1).replace(SEP_INT,"").toInt();
                    QList<QPair<int,int>>* targets = new QList<QPair<int,int>>();
                    if(strCmd.contains(SEP_MAC)) {
                        int sections = strCmd.count(SEP_MAC);
                        for(int i=0; i<sections; i+=2) {
                            int target = strCmd.section(SEP_MAC,i+1,i+1).replace(SEP_MAC,"").toInt();
                            int delay = strCmd.section(SEP_DELAY,i+1,i+1).replace(SEP_DELAY,"").toInt();
                            targets->append(QPair<int,int>(target,delay));
                        }
                    } else
                        targets->append(QPair<int,int>(TARGET_ALL,0));

                    convertTask(taskid,id,cmd,targets);

                } else if(strCmd.contains(SEP_BOOL)) {
                    QString id = strCmd.section(SEP_ID, 1, 1).replace(SEP_ID,"");
                    long taskid = strCmd.section(SEP_TASKID,1,1).replace(SEP_TASKID,"").toLong();
                    bool cmd;
                    if(strCmd.section(SEP_BOOL, 1, 1).replace(SEP_BOOL,"")=="f")
                        cmd = false;
                    else
                        cmd = true;
                    QList<QPair<int,int>>* targets = new QList<QPair<int,int>>();
                    if(strCmd.contains(SEP_MAC)) {
                        int sections = strCmd.count(SEP_MAC);
                        for(int i=0; i<sections; i+=2) {
                            int target = strCmd.section(SEP_MAC,i+1,i+1).replace(SEP_MAC,"").toInt();
                            int delay = strCmd.section(SEP_DELAY,i+1,i+1).replace(SEP_DELAY,"").toInt();
                            targets->append(QPair<int,int>(target,delay));
                        }
                    } else
                        targets->append(QPair<int,int>(TARGET_ALL,0));

                    convertTask(taskid,id,cmd,targets);

                } else if(strCmd.contains(SEP_STR)) {
                    QString id = strCmd.section(SEP_ID, 1, 1).replace(SEP_ID,"");
                    QString cmd = strCmd.section(SEP_STR, 1, 1).replace(SEP_STR,"");
                    long taskid = strCmd.section(SEP_TASKID,1,1).replace(SEP_TASKID,"").toLong();
                }
            }
        }
    }
}

void BluetoothHandler::convertTask(long taskId, QString id, int cmd, QList<QPair<int,int>>* targets) {
    for(int i=0; i<targets->size(); i++)
        emit(onTaskReceived(new TaskWrapper(taskId,id,cmd,targets->at(i).first,targets->at(i).second)));
}

void BluetoothHandler::convertTask(long taskId, QString id, bool cmd, QList<QPair<int,int>>* targets) {
    for(int i=0; i<targets->size(); i++)
        emit(onTaskReceived(new TaskWrapper(taskId,id,cmd,targets->at(i).first,targets->at(i).second)));
}


void BluetoothHandler::terminateProgram() {
    qDebug() << "BT - Removing connections";
    if(socket->isOpen()) {
        qDebug() << "BT - Was connected";
        write("Disconnecting");
    }
}

void BluetoothHandler::shutdown() {
    terminateProgram();
    QProcess process;
    process.startDetached("shutdown -P now");
}

BluetoothHandler::~BluetoothHandler() {
    terminateProgram();
}

/** TCP alerts */
void BluetoothHandler::tcpHeader() {
    write(QString(SEP_MAC).append(QString::number(serverIp)).append(SEP_MAC).append("M"));
}

void BluetoothHandler::tcpConnectionAlert(QString mac, QString ip) {
    write(QString(SEP_MAC).append(mac).append(SEP_MAC).append(ip).append(SEP_MAC).append("C"));
}

void BluetoothHandler::tcpDisconnectionAlert(QString address) {
    write(QString(SEP_MAC).append(address).append(SEP_MAC).append("D"));
}
