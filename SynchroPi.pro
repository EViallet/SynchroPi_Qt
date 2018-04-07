QT += core bluetooth network
QT -= gui

CONFIG += c++11

TARGET = SynchroPi
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    TcpHost.h \
    TcpClient.h \
    BluetoothHandler.h \
    TaskManager.h \
    TaskWrapper.h \
    GlobalVars.h \
    ServoThread.h \
    LedsController.h

SOURCES += main.cpp\
    TcpHost.cpp \
    TcpClient.cpp \
    BluetoothHandler.cpp \
    TaskManager.cpp

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -L/usr/local/include -lwiringPi -lpthread
