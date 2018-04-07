#include "TcpClient.h"

void TcpClient::init() {
    threadPauseTimer = new QTimer();
    threadPauseTimer->setInterval(PING_DELAY);
    connect(this,SIGNAL(pingSocketSignal(int)),this,SLOT(pingSocket(int)));
    connect(this,SIGNAL(initSocketsSignal()),this,SLOT(initSocket()));
    connect(this,SIGNAL(startUnpauseThreadTimer()),threadPauseTimer,SLOT(start()));
    connect(threadPauseTimer,SIGNAL(timeout()),this,SLOT(unpauseThread()));
    for(int i=0; i<MAX_PIS; i++)
        sockets->append(new QTcpSocket);
    for(int i=0; i<MAX_PIS; i++) {
        timers->append(new QTimer);
        timers->last()->setInterval(PING_TIMEOUT);
        timers->last()->setSingleShot(true);
        timers->last()->setObjectName(QString::number(i));
        connect(timers->last(), SIGNAL(timeout()), this, SLOT(onTimeout()));
    }
}

void TcpClient::run() {
    emit(initSocketsSignal());
    while(!serverListening);
    while(true) {
        for(int i=0; i<sockets->size(); i++) {
            if(sockets->at(i)->state()==QTcpSocket::ConnectedState&&sockets->at(i)->isOpen()) {
                emit(pingSocketSignal(i));
                while(!timers->at(i)->isActive());
            }
        }
        bool timersRunning = false;
        do {
            timersRunning = false;
            for(int i=0; i<timers->size(); i++)
                if(timers->at(i)->isActive())
                    timersRunning = true;
        } while(timersRunning);
        pauseThread = true;
        emit(startUnpauseThreadTimer());
        while(pauseThread);
    }
}

void TcpClient::unpauseThread() {
    pauseThread = false;
}

void TcpClient::pingSocket(int pos) {
    timers->at(pos)->start();
    sockets->at(pos)->write(QString(PING_CHAR).toLatin1());
}

void TcpClient::onTimeout() {
    QTimer* timer = (QTimer*)sender();
    sockets->at(timer->objectName().toInt())->abort();
    onSocketDisconnected(sockets->at(timer->objectName().toInt()));
}

void TcpClient::initSocket() {
    for(int i=0; i<sockets->size();i++) {
        QString temp = QString(SUBNET).replace(QString(SUBNET).lastIndexOf("0"),1,QString::number(i));
        if(!contains()&&i!=server) {
            connect(sockets->at(i),SIGNAL(connected()),this,SLOT(onSocketConnected()));
            connect(sockets->at(i),SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onSocketError(QAbstractSocket::SocketError)));
            connect(sockets->at(i),SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
            sockets->at(i)->connectToHost(QHostAddress(temp),DEFAULT_PORT);
        }
    }
}



void TcpClient::onSocketStateChanged(QAbstractSocket::SocketState state) {
    //qDebug() << "Socket state " << currentIndex <<"changed to " << state;
}

void TcpClient::onSocketConnected() {
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(DEBUG_MODE)
        qDebug() << "TcpClient - Connected to " << socket->peerAddress().toString() << " on port " << ((QTcpSocket*)sender())->peerPort();
    connect(socket,SIGNAL(readyRead()),this,SLOT(read()));
    if(socket->canReadLine())
        socket->readAll();
    if(serverListening)
        socket->write(QString(SEP_MAC).append(mac).append(SEP_MAC).toLatin1());
}

void TcpClient::onSocketError(QAbstractSocket::SocketError error) {
    QTcpSocket *socket = (QTcpSocket*)sender();
    //qDebug() << "TcpClient - Error " << error << " from socket" << socket->peerAddress().toString();
    socket->abort();
}

void TcpClient::onSocketDisconnected(QTcpSocket* socket) {
    if(DEBUG_MODE)
        qDebug() << "TcpClient - Disconnected " << socket->peerAddress().toString() << " " << socket->peerPort();
    emit(socketDisconnected(socket->peerAddress().toString()));
}

void TcpClient::read() {
    QTcpSocket *socket = (QTcpSocket*)sender();
    QString str = QString(socket->readAll());


    if(str==PING_CHAR||str.contains(PING_CHAR)) {
        if(DEBUG_MODE)
            qDebug() << "TcpClient - " << socket->peerAddress().toString().right(1) << " pinged successfully";
        timers->at(socket->peerAddress().toString().right(1).toInt())->stop();
        str = str.replace(PING_CHAR,"");
        if(str.length()==0)
            return;
    }

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

            emit(receivedTask(strCmd,SENDER_TCP));
        }
    }
    emit(receivedTask(str,SENDER_TCP));
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
            server = address.right(1).toInt();
            emit(startServer(QHostAddress(address)));
            start();
            return;
        }
    }

    connect(this,SIGNAL(onSocketError()),this,SLOT(availableAddress()));
    start();
}

void TcpClient::availableAddress() {
    int server = ((QTcpSocket*)sender())->peerAddress().toString().right(1).toInt();
    QString serverAddress = QString(SUBNET).replace(QString(SUBNET).lastIndexOf("0"),1,QString::number(server));
    QFile *file = new QFile(QString(ADDRESS_PATH));
    if(file->open(QIODevice::WriteOnly)) {
        file->write(serverAddress.toLatin1());
        file->close();
    }
    disconnect(this,SIGNAL(onSocketError()),this,SLOT(availableAddress()));
    emit(startServer(QHostAddress(serverAddress)));
}




void TcpClient::onServerOnline() {
    serverListening = true;
    for(int i=0; i<sockets->size(); i++)
        if(sockets->at(i)->state()==QTcpSocket::ConnectedState)
            sockets->at(i)->write(QString(SEP_MAC).append(mac).append(SEP_MAC).toLatin1());
}

void TcpClient::connectTo(QHostAddress a) {
    if(sockets->at(a.toString().right(1).toInt())->state()!=QTcpSocket::ConnectedState)
        sockets->at(a.toString().right(1).toInt())->connectToHost(a,DEFAULT_PORT);
}



bool TcpClient::contains() {
    for(int i=0; i<sockets->size(); i++)
        if(sockets->at(i)->peerAddress().toString().right(1).toInt())
            return true;
    return false;
}

