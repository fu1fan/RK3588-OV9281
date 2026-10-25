TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/wifiwidget.ui

HEADERS += \
    $$PWD/wifiwidget.h

SOURCES += \
    $$PWD/main.cpp    \
    $$PWD/wifiwidget.cpp

TARGET = qtdemo_wifi
