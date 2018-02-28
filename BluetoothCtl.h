#ifndef BLUETOOTHCTL_H
#define BLUETOOTHCTL_H

#include <QThread>
#include <QDebug>
#include <QProcess>


using namespace std;

class BluetoothCtl : public QThread {
    Q_OBJECT
    void run() override;
private slots:
    void read();
    void started();
    void exiting();
private:
    QProcess* process;
};

#endif // BLUETOOTHCTL_H
