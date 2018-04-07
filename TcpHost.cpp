#include "TcpHost.h"

TcpHost::TcpHost() {
    server = new QTcpServer(this);
    devices = new QList<QPair<QTcpSocket*,QString>>();
    connect(server,SIGNAL(newConnection()),this,SLOT(onClientConnected()));
    connect(server,SIGNAL(acceptError(QAbstractSocket::SocketError)),this,SLOT(onClientConnectionError(QAbstractSocket::SocketError)));
}


void TcpHost::startServer(QHostAddress a) {
    system(QString(QString("sudo ifconfig eth0 ").append(a.toString())).append(" netmask 255.255.0.0 up").toLatin1().data());

    if(server->listen(a,DEFAULT_PORT)) {
        if(DEBUG_MODE)
            qDebug() << "TcpHost - Server online on " << server->serverAddress().toString() << " on port " << server->serverPort();
        localAd = a.toString().right(1).toInt();
        emit(serverOnline());
        emit(localIp(localAd));
        devices->append(QPair<QTcpSocket*,QString>(new QTcpSocket,QString::number(localAd)));
    } else
        qWarning() << "Tcphost - Server offline : " << strerror(errno);
}

void TcpHost::requestConnectedPis(int sender) {
    emit(connectedPis(devices,sender));
}



void TcpHost::onClientConnected() {
    QTcpSocket *socket = server->nextPendingConnection();
    devices->append(QPair<QTcpSocket*,QString>(socket,""));
    if(DEBUG_MODE)
        qDebug() << "TcpHost - Client connected with address " << socket->peerAddress().toString();

    LedsController::deviceConnected(socket->peerAddress().toString().right(1).toInt());

    //connect(socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onClientStateChanged(QAbstractSocket::SocketState)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(onClientDisconnected()));
    connect(socket,SIGNAL(readyRead()),this,SLOT(read()));
}

void TcpHost::onClientConnectionError(QAbstractSocket::SocketError error) {
    if(DEBUG_MODE)
        qDebug() << "TcpHost - Client tried connecting. Got error " << error;
}

/*
void TcpHost::onClientStateChanged(QAbstractSocket::SocketState state) {
    QTcpSocket* socket = (QTcpSocket*)sender();
    //qDebug() << socket->peerAddress().toString() << " changed state : " << state;

    if(state==QAbstractSocket::UnconnectedState||state==QAbstractSocket::ClosingState)
        onClientDisconnected(socket);
}
*/

void TcpHost::onClientDisconnected() {
    onClientDisconnected((QTcpSocket*)sender());
}

void TcpHost::onClientDisconnected(QTcpSocket* socket) {
    if(DEBUG_MODE)
        qDebug() << "TcpHost - Client disconnected (" << socket->peerAddress().toString() << ")";
    if(findIndex(socket)!=-1)
        devices->removeAt(findIndex(socket));
    socket->close();

    emit(clientDisconnected(socket->peerAddress().toString().right(1)));
    LedsController::deviceDisconnected(socket->peerAddress().toString().right(1).toInt());

}




void TcpHost::read() {
    QTcpSocket *socket = (QTcpSocket*)sender();
    QString str = QString(socket->readAll());

    if(str==PING_CHAR||str.contains(PING_CHAR)) {
        if(DEBUG_MODE)
            qDebug() << "TcpHost - " << socket->peerAddress().toString().right(1) << " pinged...";
        socket->write(QString(PING_CHAR).toLatin1());
        str = str.replace(PING_CHAR,"");
        if(str.length()==0)
            return;
    }

    if(!isComplete(str)) {
        buffer.append(str);
        if(!isComplete(buffer))
            return;
        else
            str = buffer;
    }
    if(DEBUG_MODE)
        qDebug() << "TcpHost - Received : " << str << " from " << socket->peerAddress().toString();
    if(str.contains(SEP_MAC)) {
        str = str.replace(SEP_MAC,"");
        if(findIndex(socket)!=-1) {
            devices->removeAt(findIndex(socket));
            devices->append(QPair<QTcpSocket*,QString>(socket,str));
            emit(clientConnected(str,socket->peerAddress().toString().right(1)));
			emit(connectTo(socket->peerAddress()));
        }
    }
}

bool TcpHost::isComplete(QString str) {
    return str.count(SEP_DBG)==2||str.count(SEP_MAC)==2||(str.count(SEP_ID)==2&&(str.count(SEP_INT)==2||str.count(SEP_STR)==2||str.count(SEP_BOOL)==2));
}

int TcpHost::findIndex(QTcpSocket* socket) {
    for(int i=0; i<devices->size(); i++)
        if(devices->at(i).first==socket)
            return i;
    return -1;
}

void TcpHost::setCommandShift(bool shft) {
    cmdShft = shft;
    if(DEBUG_MODE)
        qDebug() << "Cmd shifting set to " << shft;
}

void TcpHost::socketDisconnected(QString a) {
    for(int i=0; i<devices->size(); i++)
        if(devices->at(i).first->peerAddress().toString()==a)
            onClientDisconnected(devices->at(i).first);
}




void TcpHost::write(QTcpSocket *socket, QString data) {
    if(socket->state()==QAbstractSocket::ConnectedState) {
        socket->write(data.toLatin1());
        if(DEBUG_MODE)
            qDebug() << "TcpHost - Msg sent : " << data << "to" << socket->peerAddress().toString();
    } else if(DEBUG_MODE)
        qDebug() << "TcpHost - Couldn't write data : state is " << socket->state() << " for address " << socket->peerAddress().toString();
}

void TcpHost::broadcast(QString data) {
    int target = TaskWrapper::getTarget(data);
    if(DEBUG_MODE)
        qDebug() << "TcpHost - Broadcasting : " << data << "to" << QString::number(target);
    if(target==-1) {
        for(int i=0; i<devices->size(); i++)
            if(devices->at(i).first->peerAddress().toString().length()>0)
                write(devices->at(i).first,data);
        return;
    }

    int pos = isDeviceConnected(target);
    if(DEBUG_MODE)
        qDebug() << "TcpHost - isDeviceConnected : " << QString::number(pos) << " - " << devices->at(pos).first->peerAddress().toString();
    if(pos!=-1)
        write(devices->at(pos).first,data);
    else if(cmdShft) {
        QList<QPair<int,int>> candidates;
        for(int i=0;i<devices->size();i++)
            candidates.append(QPair<int,int>(devices->at(i).first->peerAddress().toString().right(1).toInt(),i));
        for(int i=target; i<MAX_PIS;i++)
            if(contains(candidates,i)!=-1) {
                write(devices->at(candidates.at(contains(candidates,i)).second).first,data);
                return;
            }
        for(int i=0; i<target; i++)
            if(contains(candidates,i)!=-1) {
                write(devices->at(candidates.at(contains(candidates,i)).second).first,data);
                return;
            }
    }
}

int TcpHost::isDeviceConnected(int dev) {
    for(int j=0; j<devices->size(); j++)
        if((dev==localAd&&devices->at(j).second.length()<2)||devices->at(j).first->peerAddress().toString().right(1).toInt()==dev)
            return j;
    return -1;
}

int TcpHost::contains(QList<QPair<int,int>> candidates, int i) {
    for(int j=0; j<candidates.size(); j++)
        if(candidates.at(j).first==i)
            return j;
    return -1;
}
