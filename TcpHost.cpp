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
        qDebug() << "TcpHost - Server online on " << server->serverAddress().toString() << " on port " << server->serverPort();
        emit(serverOnline());
        emit(localIp(a.toString().right(1).toInt()));
    } else
        qWarning() << "Tcphost - Server offline : " << strerror(errno);
}

void TcpHost::requestConnectedPis() {
    emit(connectedPis(devices));
}


void TcpHost::onClientConnected() {
    QTcpSocket *socket = server->nextPendingConnection();
    devices->append(QPair<QTcpSocket*,QString>(socket,""));
    qDebug() << "TcpHost - Client connected with address " << socket->peerAddress().toString() << " on port " << socket->localPort();
    connect(socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onClientStateChanged(QAbstractSocket::SocketState)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(onClientDisconnected()));
    connect(socket,SIGNAL(readyRead()),this,SLOT(read()));
}

void TcpHost::onClientConnectionError(QAbstractSocket::SocketError error) {
    qDebug() << "TcpHost - Client tried connecting. Got error " << error;
}


void TcpHost::onClientStateChanged(QAbstractSocket::SocketState state) {/*
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(state==QAbstractSocket::UnconnectedState||state==QAbstractSocket::ClosingState)
        onClientDisconnected(socket);*/
}

void TcpHost::onClientDisconnected() {
    onClientDisconnected((QTcpSocket*)sender());
}

void TcpHost::onClientDisconnected(QTcpSocket* socket) {
    qDebug() << "TcpHost - Client disconnected (" << socket->peerAddress().toString() << ")";
    if(findIndex(socket)!=-1)
        devices->removeAt(findIndex(socket));
    socket->close();

    emit(clientDisconnected(socket->peerAddress().toString().right(1)));
}

void TcpHost::read() {
    QTcpSocket *socket = (QTcpSocket*)sender();
    QString str = QString(socket->readAll());

    if(str==PING_CHAR) {
        socket->write(PING_CHAR);
        return;
    }

    if(!isComplete(str)) {
        buffer.append(str);
        if(!isComplete(buffer))
            return;
        else
            str = buffer;
    }
    qDebug() << "TcpHost - Received : " << str << " from " << socket->peerAddress().toString();
    if(str.contains(SEP_MAC)) {
        str = str.replace(SEP_MAC,"");
        if(findIndex(socket)!=-1) {
            devices->removeAt(findIndex(socket));
            devices->append(QPair<QTcpSocket*,QString>(socket,str));
            emit(clientConnected(str,socket->peerAddress().toString().right(1)));
        }
    }
}

int TcpHost::findIndex(QTcpSocket* socket) {
    for(int i=0; i<devices->size(); i++)
        if(devices->at(i).first==socket)
            return i;
    return -1;
}


bool TcpHost::isComplete(QString str) {
    return str.count(SEP_DBG)==2||str.count(SEP_MAC)==2||(str.count(SEP_ID)==2&&(str.count(SEP_INT)==2||str.count(SEP_STR)==2||str.count(SEP_BOOL)==2));
}

void TcpHost::write(QTcpSocket *socket, QString data) {
    if(socket->isOpen()) {
        socket->write(data.toLatin1());
        qDebug() << "TcpHost - Wrote " << data << "to" << socket->peerAddress().toString();
    }
}

void TcpHost::broadcast(QString data) {
    for(int i=0; i<devices->size(); i++)
        if(TaskWrapper::isTarget(data,devices->at(i).first->peerAddress().toString().right(1).toInt()))
            write(devices->at(i).first,data);
}

void TcpHost::broadcast(long taskId) {
    QString data = CMD_CANCEL;
    data.append(SEP_TASKID).append(QString::number(taskId).append(SEP_TASKID));
    for(int i=0; i<devices->size(); i++)
        write(devices->at(i).first,data);
}

