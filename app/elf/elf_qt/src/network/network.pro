TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/networkwidget.ui

HEADERS += \
    $$PWD/networkwidget.h

SOURCES += \
    $$PWD/main.cpp    \
    $$PWD/networkwidget.cpp

TARGET =qtdemo_network
