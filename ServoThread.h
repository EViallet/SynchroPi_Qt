#ifndef SERVOTHREAD_H
#define SERVOTHREAD_H

#define PWM_PIN 1

#define PWM_INITIAL_VALUE 0
#define PWM_RANGE 1023

#define SPEED_MILLIS 200

#include <QDebug>
#include <QThread>
#include <QTimer>
#include <wiringPi.h>
#include "GlobalVars.h"


class ServoThread : public QThread {
    Q_OBJECT
    public:
        void init() {
            timer.setInterval((int)((double)(100.0-speed)*SPEED_MILLIS/100.0));
            timer.setSingleShot(false);
            connect(&timer,SIGNAL(timeout()),this,SLOT(moveServo()));
            connect(this,SIGNAL(startTimerSignal()),this,SLOT(startTimer()));
        }

        void run() override {
            emit(startTimerSignal());
            while(loop)
                loop = servoCtl != "stop";
            pairedPi = -1;
        }
    public slots:
        void setCmd(QString cmd) {
            servoCtl = cmd;
        }

        void setSyncedCmd(QString cmd) {
            pairedPi = cmd.section("-",2,2).toInt();
            if(cmd.section("_",1,1)=="S")
                servo = PWM_RANGE*1/4;
            else
                servo = PWM_INITIAL_VALUE;
            servoCtl = cmd.section("_",0,0).replace("_","");
        }

        void setSpeed(int speed) {
            timer.stop();
            timer.setInterval((int)((double)(100.0-speed)*SPEED_MILLIS/100.0));
            timer.start();
        }

        void stop() {
            servoCtl = "stop";
        }

        void onDeviceDisconnected(int pi) {
            if(pi==pairedPi)
                stop();
        }
    private slots:
        void startTimer() {
            timer.start();
        }

        void moveServo() {
            if(servoCtl=="cw") {
                // clockwise
                servo++;
                if(servo>PWM_RANGE)
                    servo = PWM_INITIAL_VALUE;
                pwmWrite(PWM_PIN, servo);
                if(DEBUG_MODE)
                    qDebug() << "Servo - " << QString::number(servo) << " - CW";
            } else if(servoCtl=="ccw") {
                // counterclockwise
                servo--;
                if(servo<PWM_INITIAL_VALUE)
                    servo = PWM_RANGE;
                pwmWrite(PWM_PIN, servo);
                if(DEBUG_MODE)
                    qDebug() << "Servo - " << QString::number(servo) << " - CCW";
            }
        }
    signals:
        void startTimerSignal();
    private:
        QTimer timer;
        QString servoCtl;
        bool loop = false;
        int speed = 50;
        int servo = PWM_INITIAL_VALUE;
        int pairedPi = -1;
};


#endif // SERVOTHREAD_H
