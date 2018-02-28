#include "BluetoothCtl.h"

void BluetoothCtl::run() {
    process = new QProcess;
    connect(process,SIGNAL(readyRead()),this,SLOT(read()));
    connect(process,SIGNAL(started()),this,SLOT(started()));
    connect(process,SIGNAL(finished(int)),this,SLOT(exiting()));
    process->start("bluetoothctl",QProcess::ReadWrite);
    process->waitForStarted(-1);
    process->waitForFinished(-1);
}

void BluetoothCtl::started() {
    qDebug() << "BTCtl - Started";
}

void BluetoothCtl::exiting() {
    qDebug() << "BTCtl - Finished";
}

void BluetoothCtl::read() {
    QString str = QString(process->readAll());
    QStringList strs = str.split("\n");
    for(int i=0; i<strs.size(); i++) {
        if(!strs.at(i).isEmpty()&&strs.at(i).contains("Connected: no")) {
            QString mac = strs.at(i).section(strs.indexOf("Device"),strs.indexOf("Connected: no")).replace("Device","").replace("Connected: no","");
            qDebug() << mac << " trying to connect.";
        }
    }
}
