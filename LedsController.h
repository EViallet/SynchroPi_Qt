#ifndef LEDSCONTROLLER_H
#define LEDSCONTROLLER_H

#define SCRIPTS_PATH "/home/pi/"

#include <QString>
#include <QProcess>

class LedsController {
    public slots:
        static void bluetoothConnected() {
            (new QProcess())->startDetached(QString("sudo ").append(SCRIPTS_PATH).append("bt_connected.py"));
        }

        static void bluetoothDisconnected() {
            (new QProcess())->startDetached(QString("sudo ").append(SCRIPTS_PATH).append("bt_disconnected.py"));
        }

        static void deviceConnected(int dev) {
            (new QProcess())->startDetached(QString("sudo ").append(SCRIPTS_PATH).append("connected.py ").append(QString::number(dev)));
        }

        static void deviceDisconnected(int dev) {
            (new QProcess())->startDetached(QString("sudo ").append(SCRIPTS_PATH).append("disconnected.py ").append(QString::number(dev)));
        }
};

#endif // LEDSCONTROLLER_H
