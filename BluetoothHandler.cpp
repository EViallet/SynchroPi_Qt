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
    if(DEBUG_MODE)
        qDebug() << "BT - Server online";
}

void BluetoothHandler::connectedPis(QList<QPair<QTcpSocket*, QString>> *devices, int sender) {
    if(sender == SENDER_BT) {
        tcpHeader();
        for(int i=0; i<devices->size();i++)
            if(devices->at(i).second.length()>2)
                tcpConnectionAlert(devices->at(i).second,devices->at(i).first->peerAddress().toString().right(1));
    }
}

void BluetoothHandler::localIp(int serverIp) {
    this->serverIp = serverIp;
}

void BluetoothHandler::onClientConnected() {
    socket = server->nextPendingConnection();

    if(DEBUG_MODE)
        qDebug() << "BT - Connected";
    LedsController::bluetoothConnected();
    emit(requestConnectedPis(SENDER_BT));
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
    if(DEBUG_MODE)
        qDebug() << "BT - Disconnected";
    LedsController::bluetoothDisconnected();
    startServer();
}


/** DATA HANDLING */
bool BluetoothHandler::isComplete(QString str) {
    return str.count(SEP_PACKET)>0&&(str.count(SEP_DBG)>=2||(str.count(SEP_ID)>=2&&(str.count(SEP_INT)>=2||str.count(SEP_STR)>=2||str.count(SEP_BOOL)>=2)));
}

bool BluetoothHandler::isCmdComplete(QString str) {
    return str.count(SEP_DBG)==2||(str.count(SEP_ID)==2&&(str.count(SEP_INT)==2||str.count(SEP_STR)==2||str.count(SEP_BOOL)==2));
}


void BluetoothHandler::write(QString data) {
    if(socket!=nullptr&&socket->state()==QBluetoothSocket::ConnectedState) {
        if(DEBUG_MODE)
            qDebug() << "BT - Writing " << data;
        socket->write(QString(SEP_PACKET).append(data).toLatin1());
    }
}


void BluetoothHandler::read() {
    QString str (socket->readAll());

    if(str=="+")
        return;

    bool valid = false;
    QString read = "";

    str.replace("+","");

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
            if(DEBUG_MODE)
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
                    int cmd = strCmd.section(SEP_INT, 1, 1).replace(SEP_INT,"").toInt();
                    QList<int>* targets = new QList<int>();
                    if(strCmd.contains(SEP_MAC)) {
                        int sections = strCmd.count(SEP_MAC);
                        for(int i=0; i<sections; i+=2) {
                            int target = strCmd.section(SEP_MAC,i+1,i+1).replace(SEP_MAC,"").toInt();
                            targets->append(target);
                        }
                    } else
                        targets->append(TARGET_ALL);

                    convertTask(id,cmd,targets);

                } else if(strCmd.contains(SEP_BOOL)) {
                    QString id = strCmd.section(SEP_ID, 1, 1).replace(SEP_ID,"");
                    bool cmd;
                    if(strCmd.section(SEP_BOOL, 1, 1).replace(SEP_BOOL,"")=="f")
                        cmd = false;
                    else
                        cmd = true;
                    QList<int>* targets = new QList<int>();
                    if(strCmd.contains(SEP_MAC)) {
                        int sections = strCmd.count(SEP_MAC);
                        for(int i=0; i<sections; i+=2) {
                            int target = strCmd.section(SEP_MAC,i+1,i+1).replace(SEP_MAC,"").toInt();
                            targets->append(target);
                        }
                    } else
                        targets->append(TARGET_ALL);

                    convertTask(id,cmd,targets);

                } else if(strCmd.contains(SEP_STR)) {
                    QString id = strCmd.section(SEP_ID, 1, 1).replace(SEP_ID,"");
                    QString cmd = strCmd.section(SEP_STR, 1, 1).replace(SEP_STR,"");
                    QList<int>* targets = new QList<int>();
                    if(strCmd.contains(SEP_MAC)) {
                        int sections = strCmd.count(SEP_MAC);
                        for(int i=0; i<sections; i+=2) {
                            int target = strCmd.section(SEP_MAC,i+1,i+1).replace(SEP_MAC,"").toInt();
                            targets->append(target);
                        }
                    } else
                        targets->append(TARGET_ALL);

                    convertTask(id,cmd,targets);

                }
            }
        }
    }
}

void BluetoothHandler::convertTask(QString id, int cmd, QList<int>* targets) {
    for(int i=0; i<targets->size(); i++)
        emit(onTaskReceived(new TaskWrapper(id,cmd,targets->at(i)),SENDER_BT));
}

void BluetoothHandler::convertTask(QString id, bool cmd, QList<int>* targets) {
    for(int i=0; i<targets->size(); i++)
        emit(onTaskReceived(new TaskWrapper(id,cmd,targets->at(i)),SENDER_BT));
}

void BluetoothHandler::convertTask(QString id, QString cmd, QList<int>* targets) {
    for(int i=0; i<targets->size(); i++)
        emit(onTaskReceived(new TaskWrapper(id,cmd,targets->at(i)),SENDER_BT));
}


void BluetoothHandler::terminateProgram() {
    if(DEBUG_MODE)
        qDebug() << "BT - Removing connections";
    if(socket->state()==QBluetoothSocket::ConnectedState) {
        if(DEBUG_MODE)
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
    write(QString(SEP_MAC).append(QString::number(serverIp)).append(SEP_MAC).append("PiMe"));
}

void BluetoothHandler::tcpConnectionAlert(QString mac, QString ip) {
    write(QString(SEP_MAC).append(mac).append(SEP_MAC).append(ip).append(SEP_MAC).append("PiCo"));
}

void BluetoothHandler::tcpDisconnectionAlert(QString address) {
    write(QString(SEP_MAC).append(address).append(SEP_MAC).append("PiDe"));
}
