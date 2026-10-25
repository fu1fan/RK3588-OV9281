TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/watchdogwidgt.ui

HEADERS += \
    $$PWD/watchdogwidgt.h

SOURCES += \
    $$PWD/main.cpp    \
    $$PWD/watchdogwidgt.cpp

TARGET = qtdemo_watchdog
