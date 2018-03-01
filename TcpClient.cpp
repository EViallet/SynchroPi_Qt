#include "TcpClient.h"

TcpClient::TcpClient() {
    sockets = new QList<QTcpSocket*>();
    pinger.setSingleShot(true);
    pinger.setInterval(TIME_BETWEEN_PINGS);
    connect(&pinger,SIGNAL(timeout()),this,SLOT(ping()));
}


void TcpClient::getMac(QString mac) {
    this->mac = mac;
    firstDiscovery();
}

void TcpClient::firstDiscovery() {
    QFile *file = new QFile(QString(ADDRESS_PATH));
    if(file->exists()&&file->open(QIODevice::ReadOnly)){
        QString address = QString(file->readAll());
        if(!QHostAddress(address).isNull()) {
            file->close();
            serverAddress = address;
            emit(startServer(QHostAddress(address)));
            return;
        }
    }

    explorer = new TcpExplorer;
    explorer->init(this,SUBNET,mac,sockets);
    connect(explorer,SIGNAL(results(QList<QTcpSocket*>)),this,SLOT(findAvailableAddress(QList<QTcpSocket*>)));
    explorer->start();
}

void TcpClient::findAvailableAddress(QList<QTcpSocket*> addresses) {
    QHostAddress *available;
    bool found = false;

    sockets->append(addresses);
    QList<QHostAddress> *existing = new QList<QHostAddress>();
    for(int j=0; j<addresses.size(); j++)
        existing->append(addresses.at(j)->peerAddress());

    int i=0;
    while(!found&&i<MAX_PIS) {
        available = new QHostAddress(QString(SUBNET).replace(QString(SUBNET).lastIndexOf("0"),1,QString::number(i)));
        if(!existing->contains(*available))
            found = true;
        else
            i++;
    }

    if(found) {
        QFile *file = new QFile(QString(ADDRESS_PATH));
        if(file->open(QIODevice::WriteOnly)) {
            file->write(available->toString().toLatin1());
            file->close();
        }
        serverAddress = *available;
        disconnect(explorer,SIGNAL(results(QList<QTcpSocket*>)),this,SLOT(findAvailableAddress(QList<QTcpSocket*>)));
        emit(startServer(*available));
    } else
        qWarning() << "No available address. " << addresses.size() << " unavailable out of " << MAX_PIS;
}

void TcpClient::ping() {
    for(int i=0; i<sockets->size(); i++) {
        sockets->at(i)->write(PING_CHAR);
        if(!sockets->at(i)->waitForReadyRead(PING_TIMEOUT)) {
            onSocketDisconnected(sockets->at(i));
            i--;
        }
    }
    pinger.start();
}


void TcpClient::discover() {
    explorer = new TcpExplorer;
    explorer->init(this,SUBNET,mac,sockets,serverAddress);
    connect(explorer,SIGNAL(results(QList<QTcpSocket*>)),this,SLOT(newSockets(QList<QTcpSocket*>)));
    explorer->start();
}


void TcpClient::newSockets(QList<QTcpSocket*> s) {
    if(s.size()>0)
        qDebug() << "TcpClient - Adding " << s.size() << " sockets";
    sockets->append(s);
    for(int i=0; i<s.size(); i++) {
        connect(s.at(i),SIGNAL(readyRead()),this,SLOT(read()));
        connect(s.at(i),SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    }
}


void TcpClient::onSocketStateChanged(QAbstractSocket::SocketState state) {
    QTcpSocket *socket = (QTcpSocket*)sender();
    if(state==QAbstractSocket::UnconnectedState)
        onSocketDisconnected(socket);
}

void TcpClient::onSocketDisconnected(QTcpSocket* socket) {
    qDebug() << "TcpClient - Disconnected " << socket->peerAddress().toString() << " " << socket->peerPort();
    sockets->removeOne(socket);
}


void TcpClient::read() {
    QTcpSocket *socket = (QTcpSocket*)sender();
    QString str = QString(socket->readAll());

    if(str==PING_CHAR)
        return;

    QString read;
    bool valid = false;

    if(!TaskWrapper::isValidTasks(str)) {
        buffer.append(str);
        if(!TaskWrapper::isValidTasks(buffer))
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
        QStringList cmds = read.split(SEP_TASK);
        for(int i=1; i<cmds.length(); i++) {
            QString strCmd = cmds.at(i);
            if(!TaskWrapper::isValidTask(strCmd)) {
                buffer.append(strCmd);
                break;
            }
            /*
            if(str.contains(QString(CMD_CANCEL))) {
                emit(receivedCancel(str.section(SEP_TASKID,1,1).replace(SEP_TASKID,"").toLong()));
                return;
            }
            */
            emit(receivedTask(strCmd));
        }
    }
    emit(receivedTask(str));
}

